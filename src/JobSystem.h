//
// Created by Lin on 2024/11/20.
//

#ifndef JOBSYSTEM_H
#define JOBSYSTEM_H
#include <condition_variable>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include <thread>

#define NUM_THREADS 8
struct JobGroup;

inline void default_complete(int begin, int end, void* global){};
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
    int data_begin;
    int data_end;
    std::mutex global_data_mutex;
    void* global_data;
    int job_count = 0;
    int job_unit_size = 0;
    int job_cur_enqueued = 0;
    void (*complete)(int data_begin, int data_end, void* global_data);
    void (*execute)(int data_begin, int data_en, void* global_data);
    JobGroup() = default;
    explicit JobGroup(int group_id,int depend_group_id ):group_id(group_id),depend_group_id(depend_group_id)
    {
    }
    JobGroup& operator=(const JobGroup& other)
    {
        throw std::runtime_error("Copy constructor not allowed");
    }

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
    void dispatch_jobs(JobGroup* job_group, int count);
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
    void alloc_jobs(int job_group_id, int job_unit_size, int data_begin, int data_end,
                    void* global_data,
                    void (*execute)(int data_begin, int data_end, void* global_data),
                    void (*complete)(int data_begin, int data_end, void* global_data));

    int create_job_group(int depend_group_id);
};


inline JobSystem JOB_SYSTEM(NUM_THREADS);




#endif //JOBSYSTEM_H
