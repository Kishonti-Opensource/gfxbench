/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <algorithm>
#include "applicationconfig.h"
#include "schemas/apidefinition.h"
#include "ng/log.h"


namespace {
    tfw::ApiDefinition gfxbench30CompatibleGL() {
        std::vector<std::string> gl33Ext;
        gl33Ext.push_back("GL_ARB_texture_storage");
        gl33Ext.push_back("GL_ARB_shading_language_420pack");
        return tfw::ApiDefinition(tfw::ApiDefinition::GL, 3, 3, gl33Ext);
    }

    tfw::ApiDefinition gfxbench40CompatibleGL() {
        std::vector<std::string> gl42Ext;
        gl42Ext.push_back("GL_ARB_shader_storage_buffer_object");
        gl42Ext.push_back("GL_ARB_explicit_uniform_location");
        gl42Ext.push_back("GL_ARB_compute_shader");
        return tfw::ApiDefinition(tfw::ApiDefinition::GL, 4, 2, gl42Ext);
    }

    tfw::ApiDefinition gles31WithAep() {
        std::vector<std::string> glesAepExt;
        glesAepExt.push_back("KHR_texture_compression_astc_ldr");
        glesAepExt.push_back("EXT_draw_buffers_indexed");
        glesAepExt.push_back("EXT_tessellation_shader");
        glesAepExt.push_back("EXT_texture_cube_map_array");
        glesAepExt.push_back("EXT_geometry_shader");
        glesAepExt.push_back("EXT_gpu_shader5");
        glesAepExt.push_back("EXT_shader_io_blocks");
        return tfw::ApiDefinition(tfw::ApiDefinition::ES, 3, 1, glesAepExt);
    }

	tfw::ApiDefinition gles31WithFPTex() {
		std::vector<std::string> glesFPTex;
		//glesAepExt.push_back("KHR_texture_compression_astc_ldr");
		//glesFPTex.push_back("GL_ARB_texture_float");
		return tfw::ApiDefinition(tfw::ApiDefinition::ES, 3, 1, glesFPTex);
	}
}


std::string ApplicationConfig::getTestListJsonName (const sysinf::SystemInfo& systemInfo) const {

	// productId == "gfxbench": these are the multi API apps for desktop and Android, where
	//     we cannot use the gfxbench_gl testlist selection logic because of Aztec
	// productId == "gfxbench_metal": the metal app hides tests based on device capabilities,
	//     thus we don't need a separate testlist
	// productId == "gfxbench_vulkan" || productId == "gfxbench_dx": these don't have community versions for 5.0
	// productId == "gfxbench_gl": doesn't have community version for 5.0, only < 5.0

	if (productId == "gfxbench" || productId == "gfxbench_metal") {
		if (isCorporateVersion)
		{
			return "testlist_corporate.json";
		}
		else
		{
			return "testlist_5.0.json";
		}
	} else if (productId == "gfxbench_vulkan" || productId == "gfxbench_dx") {
		return "testlist_corporate.json";
	} else if (productId == "gfxbench_gl") {
        if(isCorporateVersion) {
            return "testlist_corporate.json";
        }

        tfw::ApiDefinition glActual(tfw::ApiDefinition::GL,
                                    systemInfo.glInfo.majorVersion,
                                    systemInfo.glInfo.minorVersion,
                                    systemInfo.glInfo.extensions);
        tfw::ApiDefinition esActual(tfw::ApiDefinition::ES,
                                    systemInfo.glesInfo.majorVersion,
                                    systemInfo.glesInfo.minorVersion,
                                    systemInfo.glesInfo.extensions);


        tfw::ApiDefinition gl33 = gfxbench30CompatibleGL();
        tfw::ApiDefinition gl42 = gfxbench40CompatibleGL();

        tfw::ApiDefinition es30(tfw::ApiDefinition::ES, 3, 0);
        tfw::ApiDefinition es31(tfw::ApiDefinition::ES, 3, 1);
		tfw::ApiDefinition es31Aep = gles31WithAep();
		tfw::ApiDefinition es31Tex = gles31WithFPTex();

		if (glActual.isCompatibleWith(gl42) ||
			esActual.isCompatibleWith(es31Tex)) {
			return "testlist_5.0.json";

		}else if (glActual.isCompatibleWith(gl42) ||
            esActual.isCompatibleWith(es31Aep)) {
            return "testlist_4.0.json";

        } else if (esActual.isCompatibleWith(es31)) {
            return "testlist_3.1.json";

        } else if (glActual.isCompatibleWith(gl33) ||
                   esActual.isCompatibleWith(es30)) {
            return "testlist_3.0.json";

        } else {
            return "testlist_2.0.json";
        }
    } else {
        return "benchmark_tests.json";
    }
}



std::vector<std::string> ApplicationConfig::getSyncFlags (const sysinf::SystemInfo& systemInfo) const {
    std::vector<std::string> flags;

    tfw::ApiDefinition glActual(tfw::ApiDefinition::GL,
                                systemInfo.glInfo.majorVersion,
                                systemInfo.glInfo.minorVersion,
                                systemInfo.glInfo.extensions);
    tfw::ApiDefinition esActual(tfw::ApiDefinition::ES,
                                systemInfo.glesInfo.majorVersion,
                                systemInfo.glesInfo.minorVersion,
                                systemInfo.glesInfo.extensions);

    tfw::ApiDefinition gl33 = gfxbench30CompatibleGL();
    tfw::ApiDefinition gl42 = gfxbench40CompatibleGL();

    tfw::ApiDefinition es30(tfw::ApiDefinition::ES, 3, 0);
    tfw::ApiDefinition es31(tfw::ApiDefinition::ES, 3, 1);
    tfw::ApiDefinition es31Aep = gles31WithAep();

    if(platformId == "apple") {
        flags.push_back("mobile");
        flags.push_back("common_pvrtc4");
        flags.push_back("common_etc1");

		if (systemInfo.hasMetal && systemInfo.metalInfo.devices.size() > 0)
		{
			if (systemInfo.metalInfo.devices[0].astcSupport)
			{
				flags.push_back("aztec_astc");
			}
			else if (systemInfo.metalInfo.devices[0].etcSupport)
			{
				flags.push_back("aztec_etc2");
			}

			if (systemInfo.metalInfo.devices[0].tessellationSupport)
			{
				flags.push_back("metal_carchase");
				flags.push_back("metal_carchase_astc");
			}
		}

        if (esActual.isCompatibleWith(es31)) {
            flags.push_back("es31");
            flags.push_back("es31_etc2");
        }
        if (esActual.isCompatibleWith(es30)) {
            flags.push_back("es3");
            flags.push_back("es3_etc2");

        } else {
            flags.push_back("es2");
            flags.push_back("es2_pvrtc4");
        }

    } else if(platformId == "android") {
        flags.push_back("mobile");
        flags.push_back("common_etc1");

		tfw::ApiDefinition gl42 = gfxbench40CompatibleGL();
		tfw::ApiDefinition es31Tex = gles31WithFPTex();
		bool glSupportsAztec =
			glActual.isCompatibleWith(gl42) ||
			esActual.isCompatibleWith(es31Tex);

		// same logic as in ngl's ogl.cpp
		bool glesSupportsASTC = false;
		for (const std::string& extension : systemInfo.glesInfo.extensions)
		{
			if (extension.find("texture_compression_astc_ldr") != std::string::npos)
			{
				glesSupportsASTC = true;
				break;
			}
		}

		bool anyVulkanDeviceNeedsASTC = false;
		bool anyVulkanDeviceNeedsETC2 = false;
		for (const sysinf::VulkanDeviceInfo& device : systemInfo.vulkanInfo.devices)
		{
			if (device.supportsASTC)
			{
				anyVulkanDeviceNeedsASTC = true;
			}
			else if (device.supportsETC2)
			{
				anyVulkanDeviceNeedsETC2 = true;
			}
		}

		// gfxb_5.cpp chooses ASTC over ETC2 if available
		if (anyVulkanDeviceNeedsASTC || (glSupportsAztec && glesSupportsASTC))
		{
			flags.push_back("aztec_astc");
		}

		if (anyVulkanDeviceNeedsETC2 || (glSupportsAztec && !glesSupportsASTC))
		{
			flags.push_back("aztec_etc2");
		}

        if(esActual.isCompatibleWith(es31Aep)) {
            flags.push_back("es31_aep");
            flags.push_back("es31_aep_astc");
        }
        if(esActual.isCompatibleWith(es31)) {
            flags.push_back("es31");
            flags.push_back("es31_etc2");
        }
        if (esActual.isCompatibleWith(es30)) {
            flags.push_back("es3");
            flags.push_back("es3_etc2");

        } else {
            flags.push_back("es2");
            flags.push_back("es2_etc1");
        }

    } else if(platformId == "macosx" || platformId == "linux" || platformId == "windows") {

        bool has_astc = false;
        if(platformId != "macosx") // macos handled below!
        {
    		flags.push_back("aztec_dxt5");
            flags.push_back("common_dxt1");
            flags.push_back("es2_dxt1");
        }

        flags.push_back("common_etc1");

		if (systemInfo.hasMetal)
		{
			for (const sysinf::MetalDeviceInfo& device : systemInfo.metalInfo.devices)
			{
                if (device.astcSupport)
                {
                    has_astc = true;
                    flags.push_back("aztec_astc");
                    flags.push_back("common_pvrtc4");
                    flags.push_back("es2_pvrtc4");
                }
                else
                {
                    flags.push_back("aztec_dxt5");
                    flags.push_back("common_dxt1");
                    flags.push_back("es2_dxt1");
                }

				if (device.tessellationSupport)
				{
					flags.push_back("metal_carchase");
                    if (device.astcSupport)
                    {
                        flags.push_back("metal_carchase_astc");
                    }
                    else
                    {
					    flags.push_back("metal_carchase_dxt5");
                    }
				}
			}
		}

        if (glActual.isCompatibleWith(gl42))
        {
            flags.push_back("es31");
            if(has_astc)
            {
                flags.push_back("es31_aep_astc");
                flags.push_back("es31_etc2");
            }
            else
            {
                flags.push_back("es31_aep_dxt5");
                flags.push_back("es31_dxt1");
            }
            flags.push_back("es31_aep");
        }

// HACK this is needed to enabled to download manhattan test's data on Windows64 ARM platform
#ifdef _M_ARM64
        flags.push_back("es3");
        flags.push_back("es3_dxt1");
        flags.push_back("es3_dxt5");
#endif

        if (glActual.isCompatibleWith(gl33))
        {
            flags.push_back("es3");
            if(has_astc) // TODO has_etc2
            {
                flags.push_back("es3_etc2");
            }
            else
            {
                flags.push_back("es3_dxt1");
                flags.push_back("es3_dxt5");
            }
        }
        else
        {
            flags.push_back("es2");
        }

    } else {
        NGLOG_FATAL("Sync flags cannot be generated.");
        assert(false);
    }

    return flags;
}
