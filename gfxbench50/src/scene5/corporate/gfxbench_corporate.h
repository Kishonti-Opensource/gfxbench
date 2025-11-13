/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <string>

#include "schemas/descriptors.h"
#include "schemas/result.h"
#include "../gfxb_5.h"

namespace GFXB
{

template <class T>
class GFXBenchCorporateA: public GFXB::GFXBenchA<T>
{
public:
	virtual KCL::KCL_Status Init() override
	{
		KCL::KCL_Status s = GFXB::GFXBenchA<T>::Init();
		if (s != KCL::KCL_TESTERROR_NOERROR)
		{
			if (GFXB::GFXBenchA<T>::isCancelled())
			{
				setErrorResult("", tfw::Result::CANCELLED);
			}
			else
			{
				setErrorResult("INIT_FAILED", tfw::Result::FAILED);
			}
		}
		return s;
	}
	virtual SceneBase *CreateScene() override
	{
		return GFXB::GFXBenchA<T>::CreateScene();
	}

	virtual void run() override
	{
		GFXB::GFXBenchA<T>::run();
	}

private:
	
    void setErrorResult(const std::string &errmsg, tfw::Result::Status status = tfw::Result::FAILED)
    {
        tfw::Result err;
        err.setTestId(GFXBenchA<T>::name());
        err.setStatus(status);
        err.setErrorString(errmsg);
        tfw::ResultGroup resultGroup;
        resultGroup.addResult(err);
        this->CreateResults(&resultGroup);
    }
	
};

}//GFXB namespace
