//
// Created by Lin on 2024/11/20.
//

#include "JobSystem.h"
#include <shared_mutex>
#include <thread>
#include "PODPool.h"

void JobSystem::finish_job(Job* job)
{
    std::lock_guard<std::shared_mutex> lock(share_mutex);
    auto job_group = job->group;
    ++job_group->cur_finish_count;
    if (job_group->cur_finish_count == job_group->job_count)
    {
        finish_job_group(job->group->group_id);
    }
}
// 调用之前一定加锁了
void JobSystem::dispatch_jobs(JobGroup* job_group, std::size_t count)
{
    int job_count = std::min(job_group->job_count - job_group->job_cur_enqueued, count);
    assign_thread_load_temp.assign(worker_num, 0);
    for (int i = 0; i < job_count; ++i)
    {
        int min_thead_id = 0;
        int min_load = thread_loads[0];
        for (int j = 0; j < worker_num; ++j)
        {
            if (thread_loads[j] < min_load)
            {
                min_thead_id = j;
                min_load = thread_loads[j];
            }
        }
        ++thread_loads[min_thead_id];
        ++assign_thread_load_temp[min_thead_id];
    }
    int pre = 0;
    for (int i = 0; i < worker_num; ++i)
    {
        if (assign_thread_load_temp[i] == 0)
        {
            continue;
        }
        std::lock_guard<std::mutex> queue_lock(queue_mutex_vec[i]);
        for (int j = 0; j < assign_thread_load_temp[i]; ++j)
        {
            thread_work_queues[i].emplace_back();
            job_group->assign_job(thread_work_queues[i].back());
        }
        pre += assign_thread_load_temp[i];
    }
    wait_job_cond.notify_all();
}

void JobSystem::working(int thread_id)
{
    while (!is_stop)
    {
        Job job;
        // wait job ...
        {
            std::unique_lock<std::mutex> lock(queue_mutex_vec[thread_id]);
            wait_job_cond.wait(lock, [this,&job,&thread_id]
            {
                return is_stop || receive_work(thread_id, job);
            });
            if (is_stop) {
                break; // 停止线程
            }
        }
        if (is_stop) {
            break; // 停止线程
        }
        if (job.group)
        {
            job.group->execute(job.data_begin, job.data_end, job.group->global_data);

            finish_job(&job);
        }
    }
}



//运行有锁
bool JobSystem::receive_work(int thread_id, Job& job)
{
    // 尝试从自己的队列中取任务
    auto& thread_work_queue = thread_work_queues[thread_id];
    if (!thread_work_queue.empty())
    {
        auto& _job = thread_work_queue.front();
        job = _job;
        --thread_loads[thread_id];
        thread_work_queue.pop_front();
        return true;
    }

    for (int i = 0; i < worker_num; ++i)
    {
        if (i==thread_id) continue;;
        auto& _queue = thread_work_queues[i];
        if (_queue.empty()) continue;
        if (queue_mutex_vec[i].try_lock()) {

            if (!_queue.empty()) {
                auto& job_ = _queue.front();
                job = job_;
                --thread_loads[i];
                _queue.pop_front();
                queue_mutex_vec[i].unlock();
                return true;
            }
            queue_mutex_vec[i].unlock();
        }
    }
    return false;
}

//跑到这里，线程无锁，需要加锁
void JobSystem::finish_job_group(int job_group_id)
{
    if (!job_groups.contains(job_group_id))
    {
        return;
    }
    auto job_group = job_groups[job_group_id];
    if (job_group->complete)
    {
        job_group->complete(job_group->data_begin, job_group->data_end, job_group->global_data);
    }
    auto& next_job_groups = job_group_map[job_group_id];
    for (int next_job_group : next_job_groups)
    {
        if (--depend_group_count_map[next_job_group] == 0)
        {
            dispatch_job_group(job_groups[next_job_group]);
        }
    }

    job_group_map.erase(job_group_id);
    job_groups.erase(job_group_id);
    depend_group_count_map.erase(job_group_id);
    delete job_group;
    wait_job_finish_cond.notify_one();
}

void JobSystem::dispatch_job_group(JobGroup* job_group)
{
    dispatch_jobs(job_group, job_group->job_count);
}




JobSystem::JobSystem(int thread_num)
{
    worker_num = thread_num;
    thread_work_queues.resize(worker_num);
    assign_thread_load_temp.resize(worker_num);
    this->start();
}

JobSystem::~JobSystem()
{
    stop();
}


void JobSystem::submit_job_group(int group_id)
{
    std::lock_guard<std::shared_mutex> lock(share_mutex);
    auto job_group = job_groups[group_id];
    if (job_group->job_count == 0)
    {
        job_groups.erase(group_id);
        job_group_map.erase(group_id);
        depend_group_count_map.erase(group_id);
        delete job_group;
        return;
    }
    int depend_group_id = job_group->depend_group_id;
    depend_group_count_map[cur_max_job_group_id] = 0;
    if (depend_group_id > 0 && depend_group_id <= group_id && job_groups.contains(depend_group_id))
    {
        depend_group_count_map[group_id]++;
        job_group_map[depend_group_id].push_back(group_id);
    }
    else
    {
        dispatch_job_group(job_group);
    }
}


void JobSystem::wait_job_group_finish(int job_group_id)
{
    if (job_group_id == 0)
    {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(share_mutex);
    wait_job_finish_cond.wait(lock, [this,&job_group_id]
    {
        return is_stop||(job_group_id <= cur_max_job_group_id && !job_groups.contains(job_group_id));
    });
}

bool JobSystem::is_job_group_finish(int job_group_id)
{
    std::shared_lock<std::shared_mutex> lock(share_mutex);
    return (job_group_id <= cur_max_job_group_id && !job_groups.contains(job_group_id));
}

int JobSystem::create_job_group(int depend_group_id)
{
    std::lock_guard<std::shared_mutex> lock(share_mutex);
    ++cur_max_job_group_id;
    auto job_group = new JobGroup();
    job_group->group_id = cur_max_job_group_id;
    job_group->depend_group_id = depend_group_id;
    job_groups[cur_max_job_group_id] = job_group;
    job_group_map[cur_max_job_group_id] = std::vector<int>();
    depend_group_count_map[cur_max_job_group_id] = 0;
    return cur_max_job_group_id;
}

void JobSystem::alloc_jobs(int job_group_id,
                           std::size_t job_unit_size,
                           std::size_t data_begin,
                           std::size_t data_end,
                           void* global_data,
                           void (*execute)(std::size_t data_begin, std::size_t data_end,void * global_data),
                           void (*complete)(std::size_t data_begin, std::size_t data_end, void * global_data))
{
    if (job_unit_size == 0)
    {
        job_unit_size = (data_end - data_begin + 2ll * NUM_THREADS - 1ll) / (2ll * NUM_THREADS);
    }
    std::lock_guard<std::shared_mutex> lock(share_mutex);
    if (job_groups.contains(job_group_id))
    {
        auto job_group = job_groups[job_group_id];
        job_group->data_begin = data_begin;
        job_group->data_end = data_end;
        job_group->global_data = global_data;
        std::size_t job_size = (data_end - data_begin + job_unit_size - 1) / job_unit_size;
        job_group->job_unit_size = job_unit_size;
        job_group->job_count = job_size;
        job_group->complete = complete;
        job_group->execute = execute;
    }
}


bool JobSystem::stop()
{
    is_stop = true;
    wait_job_cond.notify_all(); // 唤醒所有工作线程
    wait_job_finish_cond.notify_all();
    for (auto& worker : workers) {
        if (worker.joinable())
            worker.join(); // 等待线程退出
    }
    for (auto job_group : job_groups)
    {
        if (job_group.second)
        {
            delete job_group.second;
        }
    }
    job_groups.clear();
    return true;
}

bool JobSystem::start()
{
    is_stop = false;
    for (int i = 0; i < worker_num; ++i)
    {
        workers.emplace_back([this,i]() { this->working(i); });
    }
    return true;
}
