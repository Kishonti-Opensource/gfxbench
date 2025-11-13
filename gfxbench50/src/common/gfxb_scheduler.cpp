/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_scheduler.h"

#include <Poco/Mutex.h>
#include <Poco/SynchronizedObject.h>
#include <Poco/NotificationQueue.h>
#include <algorithm>
#include <assert.h>
#include <kcl_os.h>

#include <ngl.h>

using namespace GFXB;


Task::Task(std::function<void(void)> fn, KCL::uint32 build_order) : m_is_render_task(false)
{
	m_cpu_function = fn;
	m_build_order = build_order;
	m_submit_order = 0;

	m_submitted = false;
	m_completed = false;

	m_ngl_job = 0;

	m_mutex = new Poco::Mutex();
}


Task::Task(std::function<KCL::uint32(void)> fn, KCL::uint32 build_order, KCL::uint32 submit_order) : m_is_render_task(true)
{
	m_render_function = fn;
	m_build_order = build_order;
	m_submit_order = submit_order;

	m_submitted = false;
	m_completed = false;

	m_ngl_job = 0;

	m_mutex = new Poco::Mutex();
}



void Task::Execute()
{
	if (m_is_render_task == true)
	{
		m_ngl_job = m_render_function();
	}
	else
	{
		m_cpu_function();
	}
}


KCL::uint32 Task::GetBuildOrder() const
{
	return m_build_order;
}


KCL::uint32 Task::GetSubmitOrder() const
{
	return m_submit_order;
}


bool Task::IsRenderTask() const
{
	return m_is_render_task;
}


void Task::SetCompleted(bool completed)
{
	Poco::ScopedLock<Poco::Mutex> lock(*m_mutex);
	m_completed = completed;
}


bool Task::IsCompleted()
{
	Poco::ScopedLock<Poco::Mutex> lock(*m_mutex);
	return m_completed;
}


void Task::MarkUnSubmitted()
{
	m_submitted = false;
}


bool Task::IsSubmitted() const
{
	return m_submitted;
}


void Task::Submit()
{
	if (m_is_render_task)
	{
		m_submitted = true;
	}
}


Scheduler::Scheduler()
{
	m_tasks_finalized = false;
}


Scheduler::~Scheduler()
{
	for (size_t i = 0; i < m_render_tasks.size(); i++)
	{
		delete m_render_tasks[i];
	}
}


bool Scheduler::TaskCompare(const Task *a, const Task *b)
{
	if (a->GetBuildOrder() == b->GetBuildOrder())
	{
		return a->GetSubmitOrder() < b->GetSubmitOrder();
	}
	else
	{
		return a->GetBuildOrder() < b->GetBuildOrder();
	}
}


void Scheduler::SortRenderTasks()
{
	std::stable_sort(m_render_tasks.begin(), m_render_tasks.end(), TaskCompare);
}


void Scheduler::ScheduleTask(Task *task)
{
	if (m_tasks_finalized)
	{

		assert(0);
	}
	else
	{
		m_render_tasks.push_back(task);
	}
}


void Scheduler::FinalizeTasks()
{
	if (m_tasks_finalized)
	{
		INFO("Render task list is already finalized!");
		assert(0);
	}

	SortRenderTasks();

	m_tasks_finalized = true;
}


void SingleThreadedScheduler::Execute()
{
	if (m_tasks_finalized == false)
	{
		INFO("Render task list is not finalized!");
		assert(0);
	}

	for (size_t i = 0; i < m_render_tasks.size(); i++)
	{
		m_render_tasks[i]->Execute();
		m_render_tasks[i]->Submit();
	}
}


void SingleThreadedScheduler::Shutdown()
{
	// Nothing to implement here
}


RenderThread::RenderThread(Poco::NotificationQueue *queue, Poco::SynchronizedObject *sync_object)
{
	m_queue = queue;
	m_sync_object = sync_object;
}


RenderThread::~RenderThread()
{
}


void RenderThread::run()
{
	while (true)
	{
		RenderNotification *notification = (RenderNotification*)m_queue->waitDequeueNotification();

		Task *task = notification->m_render_task;
		if (task == nullptr)
		{
			// Thread is closing
			return;
		}

		task->Execute();
		task->SetCompleted(true);
		m_sync_object->notify();
	}
}


MultiThreadedScheduler::MultiThreadedScheduler(KCL::uint32 thread_count) : m_thread_count(thread_count)
{
	m_sync_object = new Poco::SynchronizedObject();
	m_queue = new Poco::NotificationQueue();
}


MultiThreadedScheduler::~MultiThreadedScheduler()
{
	for (size_t i = 0; i < m_threads.size(); i++)
	{
		delete m_threads[i];
	}

	for (size_t i = 0; i < m_quit_notifications.size(); i++)
	{
		delete m_quit_notifications[i];
	}

	for (size_t i = 0; i < m_render_notifications.size(); i++)
	{
		delete m_render_notifications[i];
	}

	delete m_sync_object;
	delete m_queue;
}


void MultiThreadedScheduler::FinalizeTasks()
{
	Scheduler::FinalizeTasks();

	INFO("MultiThreadedSceduler - Creating threads: %d", m_thread_count);
	m_threads.resize(m_thread_count);
	for (size_t i = 0; i < m_thread_count; i++)
	{
		RenderThread *thread = new RenderThread(m_queue, m_sync_object);
		thread->start(*thread);
		m_threads.push_back(thread);

		m_quit_notifications.push_back(new RenderNotification(nullptr));
	}

	for (size_t i = 0; i < m_render_tasks.size(); i++)
	{
		m_render_notifications.push_back(new RenderNotification(m_render_tasks[i]));
	}
}


void MultiThreadedScheduler::Execute()
{
	if (m_tasks_finalized == false)
	{
		INFO("Render task list is not finalized!");
		assert(0);
	}

	m_sync_object->lock();
	{
		for (size_t i = 0; i < m_render_notifications.size(); i++)
		{
			m_render_notifications[i]->m_render_task->SetCompleted(false);
			m_queue->enqueueNotification(m_render_notifications[i]);
		}

		bool completed = false;
		do
		{
			m_sync_object->wait();

			completed = true;
			for (size_t i = 0; i < m_render_tasks.size(); i++)
			{
				if (m_render_tasks[i]->IsCompleted() == false)
				{
					completed = false;
					break;
				}
			}

		} while (completed == false);
	}
	m_sync_object->unlock();

	for (size_t i = 0; i < m_render_tasks.size(); i++)
	{
		m_render_tasks[i]->Submit();
	}
}


void MultiThreadedScheduler::Shutdown()
{
	INFO("MultiThreadedSceduler - Shutdown");
	for (size_t i = 0; i < m_quit_notifications.size(); i++)
	{
		m_queue->enqueueUrgentNotification(m_quit_notifications[i]);
	}

	INFO("MultiThreadedSceduler - Joining threads...");
	for (size_t i = 0; i < m_threads.size(); i++)
	{
		INFO("MultiThreadedSceduler - Joining thread: %d/%d", i, m_threads.size());
		m_threads[i]->join();
	}
}



MultiThreadedOnDemandScheduler::MultiThreadedOnDemandScheduler(KCL::uint32 thread_count)
: MultiThreadedScheduler(thread_count)
{

}


MultiThreadedOnDemandScheduler::~MultiThreadedOnDemandScheduler()
{

}


void MultiThreadedOnDemandScheduler::Execute()
{
	if (m_tasks_finalized == false)
	{
		INFO("Render task list is not finalized!");
		assert(0);
	}

	KCL::uint32 m_max_submitted_execution_group_end = 0;

	m_sync_object->lock();
	{
		for (size_t i = 0; i < m_render_notifications.size(); i++)
		{
			m_render_notifications[i]->m_render_task->SetCompleted(false);
			m_queue->enqueueNotification(m_render_notifications[i]);
		}

		for (size_t i = 0; i < m_render_tasks.size(); i++)
		{
			m_render_tasks[i]->MarkUnSubmitted();
		}

		bool completed = false;
		do
		{
			m_sync_object->wait();

			completed = true;
			KCL::uint32 min_uncompleted_order = 0;
			for (size_t i = m_max_submitted_execution_group_end; i < m_render_tasks.size(); i++)
			{
				min_uncompleted_order = m_render_tasks[i]->GetSubmitOrder();
				if (m_render_tasks[i]->IsCompleted() == false)
				{
					completed = false;
					break;
				}
			}


			size_t j = m_max_submitted_execution_group_end;
			for (; j < m_render_tasks.size(); j++)
			{
				if (m_render_tasks[j]->GetSubmitOrder() >= min_uncompleted_order)
				{
					m_max_submitted_execution_group_end = (KCL::uint32)j;
					break;
				}

				if (!m_render_tasks[j]->IsSubmitted())
				{
					m_render_tasks[j]->Submit();
				}
			}


			for (; j < m_render_tasks.size(); j++)
			{
				if (m_render_tasks[j]->GetSubmitOrder() > min_uncompleted_order)
				{
					break;
				}

				if (m_render_tasks[j]->IsCompleted() && !m_render_tasks[j]->IsSubmitted())
				{
					m_render_tasks[j]->Submit();
				}
			}


		} while (completed == false);
	}
	m_sync_object->unlock();
}



