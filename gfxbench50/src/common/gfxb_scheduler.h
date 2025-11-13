/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_SCHEDULER
#define GFXB_SCHEDULER

#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/Notification.h>
#include <functional>
#include <vector>
#include <kcl_base.h>

namespace Poco
{
	class Thread;
	class Mutex;
	class SynchronizedObject;
	class NotificationQueue;
}

namespace GFXB
{
	class Task
	{
	public:
		Task(std::function<void(void)> fn, KCL::uint32 build_order);
		Task(std::function<KCL::uint32(void)> fn, KCL::uint32 build_order, KCL::uint32 submit_order);
		virtual ~Task() {}

		void Execute();

		KCL::uint32 GetBuildOrder() const;
		KCL::uint32 GetSubmitOrder() const;

		bool IsRenderTask() const;

		void SetCompleted(bool completed); // Thread safe
		bool IsCompleted(); // Thread safe

		void Submit();
		void MarkUnSubmitted();
		bool IsSubmitted() const;

	private:
		const bool m_is_render_task;

		std::function<void(void)> m_cpu_function;
		std::function<KCL::uint32(void)> m_render_function;

		KCL::uint32 m_build_order;
		KCL::uint32 m_submit_order;

		KCL::uint32 m_ngl_job;

		Poco::Mutex *m_mutex; // Guard for m_completed
		bool m_completed;
		bool m_submitted;
	};


	class RenderNotification : public Poco::Notification
	{
	public:
		RenderNotification(Task *task)
		{
			m_render_task = task;
		}

		Task *m_render_task;
	};


	class RenderThread : public Poco::Thread, public Poco::Runnable
	{
	public:
		RenderThread(Poco::NotificationQueue *queue, Poco::SynchronizedObject *sync_object);
		virtual ~RenderThread();

		void run() override;

	private:
		Poco::NotificationQueue *m_queue;

		// Wakes up the main thread when the job is completed
		Poco::SynchronizedObject *m_sync_object;
	};


	class Scheduler
	{
	public:
		Scheduler();
		virtual ~Scheduler();

		virtual void Execute() = 0;
		virtual void Shutdown() = 0;

		void ScheduleTask(Task *task);
		virtual void FinalizeTasks();

	protected:
		static bool TaskCompare(const Task *a, const Task *b);
		void SortRenderTasks();

		bool m_tasks_finalized;
		std::vector<Task*> m_render_tasks;
	};


	class SingleThreadedScheduler : public Scheduler
	{
	public:
		void Execute() override;
		void Shutdown() override;
	};


	class MultiThreadedScheduler : public Scheduler
	{
	public:
		MultiThreadedScheduler(KCL::uint32 thread_count);
		virtual ~MultiThreadedScheduler();

		void FinalizeTasks() override;
		void Execute() override;
		void Shutdown() override;

	protected:
		Poco::NotificationQueue *m_queue;
		Poco::SynchronizedObject *m_sync_object; // Sync object for the worker threads to weak up the main thread

		const KCL::uint32 m_thread_count;
		std::vector<RenderThread*> m_threads;

		std::vector<RenderNotification*> m_render_notifications;
		std::vector<RenderNotification*> m_quit_notifications;
	};


	class MultiThreadedOnDemandScheduler : public MultiThreadedScheduler
	{
	public:
		MultiThreadedOnDemandScheduler(KCL::uint32 thread_count);
		virtual ~MultiThreadedOnDemandScheduler();

		void Execute() override;
	};
};

#endif

