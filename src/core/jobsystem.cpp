#include "hge_impl.h"

void HGE_Impl::JS_Start(hgeJobThreadCallback thread_start, hgeJobThreadCallback thread_end)
{
	pJobSystem = new utils::JobSystem(thread_start, thread_end);
	pJobSystem->adopt();
}

void CALL HGE_Impl::JS_Shutdown()
{
	pJobSystem->emancipate();
	delete pJobSystem;
	pJobSystem = NULL;
}

int CALL HGE_Impl::JS_GetWorkerThreadCount()
{
	if (!pJobSystem) return 0;
	return pJobSystem->getThreadCount();
}

int CALL HGE_Impl::JS_GetThreadId()
{
	if (!pJobSystem) return 0;
	return pJobSystem->getThreadId();
}

HJOB CALL HGE_Impl::JS_CreateJob(HJOB parent)
{
	if (!pJobSystem) return NULL;
	return (HJOB)pJobSystem->createJob();
}

HJOB CALL HGE_Impl::JS_CreateJob(hgeJobCallback jobCallback, const hgeJobPayload& payload, HJOB parent)
{
	if (!pJobSystem) return NULL;
	return (HJOB)pJobSystem->createJob((utils::JobSystem::Job*)parent, [this, jobCallback, payload](utils::JobSystem& js, utils::JobSystem::Job* parent)
		{
			jobCallback(this, (HJOB)parent, payload);
		});
}

void CALL HGE_Impl::JS_Run(HJOB job)
{
	if (!pJobSystem) return;
	pJobSystem->run((utils::JobSystem::Job*)job);
}

void CALL HGE_Impl::JS_RunAndWait(HJOB job)
{
	if (!pJobSystem) return;
	pJobSystem->runAndWait((utils::JobSystem::Job*)job);
}

