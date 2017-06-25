#include "thread.h"


namespace txh{

template struct SafeQueue<Task>;

template <typename T>
bool SafeQueue<T>::push(T &&v)
{ 
	lock_guard<mutex> lk(*this);
	if(m_exit || (m_capacity && m_items.size() >= m_capacity))
		return false;
	m_items.push_back(move(v));
	m_ready.notify_one();
	return true;
}

template<typename T>
void SafeQueue<T>::wait_ready(unique_lock<mutex> &lock,int waitMs)
{
	if(m_exit || !m_items.empty())
		return;
	
	if(waitMs == wait_infinite)
		m_ready.wait(lock,[this]{ return m_exit || !m_items.empty() ;});
	else if(waitMs > 0){
		auto tp = chrono::steady_clock::now() + chrono::milliseconds(waitMs);
		while(m_ready.wait_until(lock,tp) == cv_status::timeout && m_items.empty() && !m_exit)
			;
	}
}

template <typename T>
T SafeQueue<T>::pop_wait(int waitMs)
{
	unique_lock<mutex> lk(*this);
	wait_ready(lk,waitMs);
	if(m_items.empty())
		return T();
	T r = move(m_items.front());
	m_items.pop_front();
	return r;
}

template <typename T>
bool SafeQueue<T>::pop_wait(T* v,int waitMs)
{
	unique_lock<mutex> lk(*this);
	wait_ready(lk,waitMs);
	if(m_items.empty())
		return false;
	*v = move(m_items.front());
	m_items.pop_front();
	return true;
}

template <typename T>
size_t SafeQueue<T>::size()
{
	lock_guard<mutex> lk(*this);
	return m_items.size();	
}

template <typename T>
void SafeQueue<T>::exit()
{
	m_exit = true;
	lock_guard<mutex> lk(*this);
	m_ready.notify_all();
}


ThreadPool::ThreadPool(int threads,int taskCapacity , bool start)
:m_threads(threads),m_tasks(taskCapacity)
{
	if(start){
		this->start();
	}
}

ThreadPool::~ThreadPool()
{
	assert(m_tasks.exited());
	if(m_tasks.size()){
		fprintf(stderr,"%ld tasks not processed when threadpoll exit\n ",m_tasks.size());
	}	
}
void ThreadPool::start()
{
	for(auto &th : m_threads){
		thread t(
			[this]{
				while(!m_tasks.exited()){
					Task task;
					if(m_tasks.pop_wait(&task)){
						task();
					}
				}	
			}
		);
		th.swap(t);
	}
}
void ThreadPool::join()
{
	for(auto &t:m_threads){
		t.join();
	}	
}

bool ThreadPool::addTask(Task &&task)
{
	return m_tasks.push(move(task));
}

ThreadPool& ThreadPool::exit()
{ 	
	m_tasks.exit();
	return *this;
}
	
}
