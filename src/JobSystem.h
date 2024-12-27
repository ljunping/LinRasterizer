//
// Created by Lin on 2024/11/20.
//

#ifndef JOBSYSTEM_H
#define JOBSYSTEM_H
#include <condition_variable>
#include <shared_mutex>
#include <unordered_map>
#include <deque>
#include <utility>
#include <vector>
#include <thread>
#include "debug.h"

#define NUM_THREADS 8
#define PARALLEL_SORT_THR 1024

struct JobGroup;

inline void default_complete(std::size_t begin, std::size_t end, void* global){};

struct Job
{
    int data_begin;
    int data_end;
    JobGroup* group;
};

struct JobGroup
{
    int group_id;
    int depend_group_id;
    int cur_finish_count{0};
    int cur_execute_job = 0;
    std::size_t data_begin;
    std::size_t data_end;
    std::mutex global_data_mutex;
    void* global_data;
    std::size_t job_count = 0;
    std::size_t job_unit_size = 0;
    std::size_t job_cur_enqueued = 0;
    void (*complete)(std::size_t data_begin, std::size_t data_end, void* global_data);
    void (*execute)(std::size_t data_begin, std::size_t data_en, void* global_data);
    JobGroup() = default;
    explicit JobGroup(int group_id,int depend_group_id ):group_id(group_id),depend_group_id(depend_group_id)
    {
    }

    JobGroup& operator=(const JobGroup& other) = delete;

    void assign_job(Job& job)
    {
        job.data_begin = data_begin + job_cur_enqueued * job_unit_size;
        job.data_end = std::min(data_begin + (job_cur_enqueued + 1) * job_unit_size, data_end);
        job.group=this;
        ++job_cur_enqueued;
    }
};



class JobSystem
{
    std::shared_mutex share_mutex;
    std::condition_variable_any wait_job_cond;
    std::condition_variable_any wait_job_finish_cond;
    int cur_max_job_group_id;
    std::vector<std::deque<Job>> thread_work_queues;
    std::mutex queue_mutex_vec[NUM_THREADS];
    std::unordered_map<int, JobGroup*> job_groups;
    std::unordered_map<int, std::vector<int>> job_group_map;
    std::unordered_map<int, int> depend_group_count_map;
    int worker_num=0;
    std::vector<int> assign_thread_load_temp;
    std::atomic<int> thread_loads[NUM_THREADS]; // 每个线程当前的任务数
    std::vector<std::thread> workers;
    void finish_job(Job* job);
    void dispatch_jobs(JobGroup* job_group, std::size_t count);
    void working(int thread_id);
    bool receive_work(int thread_id, Job& job);
    void finish_job_group(int job_group_id);
    void dispatch_job_group(JobGroup* job_group);
    bool is_stop;
public:
    explicit JobSystem(int thread_num);
    ~JobSystem();
    void submit_job_group(int job_group_id);
    void wait_job_group_finish(int job_group_id);
    bool is_job_group_finish(int job_group_id);
    bool stop();
    bool start();
    void alloc_jobs(int job_group_id, std::size_t job_unit_size, std::size_t data_begin, std::size_t data_end,
                    void* global_data,
                    void (*execute)(std::size_t data_begin, std::size_t data_end, void* global_data),
                    void (*complete)(std::size_t data_begin, std::size_t data_end, void* global_data));

    int create_job_group(int depend_group_id);
};
inline JobSystem JOB_SYSTEM(NUM_THREADS);

template <class RandomAccess, class Comp>
void sort(std::size_t data_begin, std::size_t data_end, void* global_data)
{
    std::tuple<RandomAccess, RandomAccess, Comp>& data = (*static_cast<std::tuple<RandomAccess, RandomAccess, Comp>*>(
        global_data));
    auto _begin_ptr = std::get<0>(data);
    std::sort(_begin_ptr + data_begin, _begin_ptr + data_end, std::get<2>(data));
}

#include <queue>
#include <type_traits>
template <typename T>
using DereferenceType = typename std::remove_reference<decltype(*std::declval<T>())>::type;

template <class RandomAccess>
using value_type = DereferenceType<RandomAccess>;

template <class RandomAccess, class Comp>
void merge(std::size_t data_begin, std::size_t data_end, void* global_data)
{
    auto& data = (*static_cast<std::tuple<RandomAccess, RandomAccess, Comp, std::vector<std::vector<int>>*, int,int>*>(
        global_data));
    auto _begin_ptr = std::get<0>(data);
    auto _end_ptr = std::get<1>(data);
    auto& comp = std::get<2>(data);
    auto& _merge_index_array = *std::get<3>(data);
    auto& merge_job_index = std::get<4>(data);
    auto& _sort_count = std::get<5>(data);
    auto& pre_index = _merge_index_array[merge_job_index];
    auto& cur_index = _merge_index_array[merge_job_index + 1];
    auto _pre_index = pre_index;
    std::vector<value_type<RandomAccess>> temp(data_end - data_begin);
    for (int i = data_begin; i < data_end; ++i)
    {
        int min_i = -1;
        for (int j = 0; j < _sort_count; ++j)
        {
            if (_pre_index[j] < cur_index[j])
            {
                if (min_i < 0 || comp(_begin_ptr[_pre_index[j]], _begin_ptr[min_i]))
                {
                    min_i = j;
                }
            }
        }
        temp[i - data_begin] = *(_begin_ptr + _pre_index[min_i]);
        ++_pre_index[min_i];
    }
    for (int i = data_begin; i < data_end; ++i)
    {
        *(_begin_ptr + i) = temp[i - data_begin];
    }
}
template<class Comp>
using DecayComp = typename std::decay<Comp>::type;

template <class RandomAccess, class Comp>
void parallel_sort(RandomAccess begin, RandomAccess end, Comp comp)
{
    int count = end - begin;
    if (count <= 1)
    {
        return;
    }
    if constexpr (NUM_THREADS == 1)
    {
        std::sort(begin, end, comp);
        return;
    }
    if (count < PARALLEL_SORT_THR)
    {
        std::sort(begin, end, comp);
        return;
    }
    std::sort(begin, end, comp);
    return;
    int sort_job_count = NUM_THREADS;
    auto block_size = (count + sort_job_count - 1) / sort_job_count;
    auto sort_job_id = JOB_SYSTEM.create_job_group(0);
    std::tuple<RandomAccess, RandomAccess, Comp> data(begin, end, comp);
    JOB_SYSTEM.alloc_jobs(sort_job_id, block_size, 0, end - begin, &data, sort<RandomAccess, Comp>, default_complete);
    JOB_SYSTEM.submit_job_group(sort_job_id);
    JOB_SYSTEM.wait_job_group_finish(sort_job_id);
    int merge_job_count = 2;
    auto merge_block_size = (block_size + merge_job_count - 1) / merge_job_count;
    std::vector<std::vector<int>> merge_index_array(merge_job_count * sort_job_count + 1,
                                                    std::vector<int>(sort_job_count, 0));
    std::vector<std::tuple<RandomAccess, RandomAccess, DecayComp<Comp>, std::vector<std::vector<int>>*, int, int>>
        merge_data(
            merge_job_count * sort_job_count, std::make_tuple(begin, end, comp, nullptr, 0, 0));

    int merge_job_index = 0;
    int merge_start_index = 0;
    for (int _id = 0; _id < sort_job_count; ++_id)
    {
        merge_index_array[0][_id] = std::min(block_size * _id, count);
    }
    std::vector<int> job_groups;
    for (int _id = 0; _id < sort_job_count; ++_id)
    {
        for (int i = 0; i < merge_job_count; ++i)
        {
            int end_index = merge_start_index;
            auto& pre_index_arrar = merge_index_array[merge_job_index];
            auto l = pre_index_arrar[_id];
            auto r = std::min(l + (i + 1) * merge_block_size, count);
            if (r == l)
            {
                break;
            }
            end_index += r - l;
            auto& index_array = merge_index_array[merge_job_index + 1];
            for (int j = 0; j < _id; ++j)
            {
                index_array[j] = pre_index_arrar[j];
            }
            index_array[_id] = r;
            auto find_v = *(begin + r - 1);
            for (int j = _id + 1; j < sort_job_count; ++j)
            {
                auto sl = pre_index_arrar[j];
                auto sr = std::min(pre_index_arrar[j] + (j + 1) * block_size, count);
                auto var = std::lower_bound(begin + sl, begin + sr, find_v, comp) - begin;
                end_index += var - sl;
                index_array[j] = var;
            }
            std::get<0>(merge_data[merge_job_index]) = begin;
            std::get<1>(merge_data[merge_job_index]) = end;
            std::get<3>(merge_data[merge_job_index]) = &merge_index_array;
            std::get<4>(merge_data[merge_job_index]) = merge_job_index;
            std::get<5>(merge_data[merge_job_index]) = sort_job_count;
            auto job_group_id = JOB_SYSTEM.create_job_group(0);
            JOB_SYSTEM.alloc_jobs(job_group_id,
                                  end_index - merge_start_index,
                                  merge_start_index, end_index,
                                  &merge_data[merge_job_index],
                                  merge<RandomAccess, Comp>,
                                  default_complete);
            JOB_SYSTEM.submit_job_group(job_group_id);
            merge_job_index++;
            merge_start_index = end_index;
            job_groups.push_back(job_group_id);
        }
    }
    for (int job_group : job_groups)
    {
        JOB_SYSTEM.wait_job_group_finish(job_group);
    }
}

template <class RandomAccess>
void parallel_sort(RandomAccess begin, RandomAccess end)
{
    parallel_sort(begin, end, std::less<>());
}







#endif //JOBSYSTEM_H
