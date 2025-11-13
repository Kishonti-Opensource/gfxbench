/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import java.security.InvalidParameterException;

import net.kishonti.swig.Result;
import net.kishonti.swig.ResultGroup;
import net.kishonti.swig.TestFactory;

public class TestUtils {

	public static JTestInterface crateTest(String name, String config) throws InvalidParameterException {
		JTestInterface t = null;
		if (name != null) {
			TestFactory factory = TestFactory.test_factory(name);
			if (factory.valid()) {
				t = factory.create_test();
				if (t != null) {
					if (config != null) {
						t.setConfig(config);
					}
				} else {
					throw new InvalidParameterException("Factory failed to create test: " + name);
				}
			} else {
				throw new InvalidParameterException("Failed to create test factory: " + name);
			}				
		}
		return t;
	}
	
	public static class ResultJson {
		public Result[] results = new Result[0];
	}
	
	public static ResultGroup createSingleResultList(Result r) {
		ResultGroup ret = new ResultGroup();
		ret.addResult(r);
		return ret;
	}
	
	public static Result createCancelledResult(String test_id, String result_id) {
		Result res = new Result();
		res.setTestId(test_id);
		res.setResultId(result_id);
		res.setStatus(Result.Status.CANCELLED);
		return res;
	}

	public static Result createFailedResult(String test_id, String result_id, String error) {
		Result res = new Result();
		res.setTestId(test_id);
		res.setResultId(result_id);
		res.setStatus(Result.Status.FAILED);
		res.setErrorString(error);
		return res;
	}
}
