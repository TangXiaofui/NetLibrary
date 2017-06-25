#ifndef THREAD_H
#define THREAD_H
#include <assert.h>
#include <chrono>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <limits>
#include <functional>
#include "util.h"
#include <list>
#include <atomic>
namespace txh{

using namespace std;
template <typename T>
struct SafeQueue:private mutex,private noncopyable{
	static const int wait_infinite = numeric_limits<int>::max();
	SafeQueue(size_t capacity = 0):m_capacity(capacity),m_exit(false){}
	bool push(T &&v);
	T pop_wait(int waitMs = wait_infinite);
	bool pop_wait(T* v,int waitMs = wait_infinite);
	
	size_t size();
	void exit();
	bool exited(){  return m_exit ; }
private:
	list<T> m_items;
	condition_variable m_ready;
	size_t m_capacity;
	atomic<bool> m_exit;
	void wait_ready(unique_lock<mutex> &lock,int waitMs);
};


typedef function<void()> Task;
extern template struct SafeQueue<Task>;

struct ThreadPool:private noncopyable{
	ThreadPool(int threads,int taskCapacity = 0, bool start = true);
	~ThreadPool();

	void start();
	ThreadPool& exit();
	void join();

	bool addTask(Task &&task);
	bool addTask(Task &task){ return addTask(move(task)); }
	size_t taskSize() { return m_tasks.size(); }
private:
	SafeQueue<Task> m_tasks;
	vector<thread> m_threads;

};

	
	
}




#endif
