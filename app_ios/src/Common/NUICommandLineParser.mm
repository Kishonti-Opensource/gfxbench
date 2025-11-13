/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "NUICommandLineParser.h"
#import "Utils/NSFileManagerLocations.h"
#import <UIKit/UIKit.h>
#import <Poco/Util/OptionSet.h>
#import <Poco/Util/OptionProcessor.h>
#import <Poco/Util/IntValidator.h>
#import <Poco/Util/HelpFormatter.h>
#import <iostream>

class PocoObserver {
public:
    virtual ~PocoObserver() {};
    NUICommandLineParser *delegate;
    void addTest(const std::string &key, const std::string &value);
    void handleHelp(const std::string &key, const std::string &value);
    void handleVerbose(const std::string &key, const std::string &value);
    void handleExtraArgs(const std::string &key, const std::string &value);
};

@interface NUICommandLineParser()

@property (nonatomic, strong) NSMutableArray *testIds;
@property (nonatomic, strong) NSMutableDictionary *rawn;
@property (nonatomic, strong) NSMutableDictionary *raws;
@property (nonatomic, strong) NSMutableDictionary *rawz;
@property (nonatomic, strong) NSMutableDictionary *config;

- (void)addTestWithKey:(std::string)key Value:(std::string)value;
- (void)handleHelpWithKey:(std::string)key Value:(std::string)value;
- (void)handleVerboseWithKey:(std::string)key Value:(std::string)value;
- (void)handleExtraArgsWithKey:(std::string)key Value:(std::string)value;


@end

@implementation NUICommandLineParser {
    Poco::Util::OptionSet optionSet;
    BOOL helpNeeded;
    PocoObserver observer;
    tfw::DescriptorList TFWDescriptors;
}

- (void)fillOptionSet:(Poco::Util::OptionSet *)options {
    Poco::Util::Option basePath("base_path", "b", "specify the base directory");
    basePath
    .required(false)
    .argument("dir", true)
    .binding("base_path");
    options->addOption(basePath);

    Poco::Util::Option width("width", "w", "specify the window width");
    width
    .required(false)
    .argument("value", true)
    .binding("width").validator(new Poco::Util::IntValidator(1,4096));
    options->addOption(width);

    Poco::Util::Option height("height", "h", "specify the window height");
    height
    .required(false)
    .argument("value", true)
    .binding("height").validator(new Poco::Util::IntValidator(1,4096));
    options->addOption(height);

    Poco::Util::Option test_id("test_id", "t", "comma separated list of test ids");
    test_id
    .required(false)
    .repeatable(true)
    .argument("value", true)
    .callback(Poco::Util::OptionCallback<PocoObserver>(&observer, &PocoObserver::addTest));
    options->addOption(test_id);

    Poco::Util::Option help("help", "?", "show help");
    help.callback(Poco::Util::OptionCallback<PocoObserver>(&observer, &PocoObserver::handleHelp));
    options->addOption(help);

    Poco::Util::Option resultdir("resultdir", "", "set directory where to save the results");
    resultdir
    .required(false)
    .argument("result_path", true)
    .binding("resultdir");
    options->addOption(resultdir);

//    Poco::Util::Option gl_api("gl_api", "", "specify OpenGL (ES) context type. Supported values: desktop_core, desktop_compat, gles");
//    gl_api
//    .required(false)
//    .argument("API", true)
//    .binding("gl_api");
//    options->addOption(gl_api);

//    Poco::Util::Option pluginPath("plugin_path", "", "specify the plugins directory");
//    pluginPath
//    .required(false)
//    .argument("dir", true)
//    .binding("plugin_path");
//    options->addOption(pluginPath);

//    Poco::Util::Option fullscreen("fullscreen", "", "run the application in full screen mode");
//    fullscreen
//    .required(false)
//    .argument("value", true)
//    .binding("fullscreen").validator(new Poco::Util::IntValidator(0,1));
//    options->addOption(fullscreen);

//    Poco::Util::Option autotest("autotest", "", "run all tests matching filter");
//    autotest
//    .required(false)
//    .binding("autotest");
//    options->addOption(autotest);

//    Poco::Util::Option filter("filter", "", "set test_id filter regular expression");
//    filter
//    .required(false)
//    .argument("regex")
//    .binding("filter");
//    options->addOption(filter);

//    Poco::Util::Option debug_postfix("debug_postfix", "", "debug postfix for the plugin file name");
//    debug_postfix
//    .required(false)
//    .argument("_d", true)
//    .binding("debug_postfix");
//    options->addOption(debug_postfix);

//    Poco::Util::Option force("force", "", "forces the application to continue if some tests fail");
//    force
//    .required(false)
//    .repeatable(false)
//    .binding("force");
//    options->addOption(force);

//    Poco::Util::Option verbose("verbose", "v", "set verbose level");
//    verbose
//    .required(false)
//    .repeatable(true)
//    .callback(Poco::Util::OptionCallback<PocoObserver>(&observer, &PocoObserver::handleVerbose));
//    options->addOption(verbose);

//    Poco::Util::Option cldevice("cl_device", "c", "select OpenCL device index");
//    cldevice
//    .required(false)
//    .argument("index", true)
//    .binding("cl_device").validator(new Poco::Util::IntValidator(0,8));
//    options->addOption(cldevice);

    Poco::Util::Option extra_args_ei("ei", "", "integer arg");
    extra_args_ei
    .required(false)
    .repeatable(true)
    .argument("value", true)
    .callback(Poco::Util::OptionCallback<PocoObserver>(&observer, &PocoObserver::handleExtraArgs));
    options->addOption(extra_args_ei);

    Poco::Util::Option extra_args_ef("ef", "", "float arg");
    extra_args_ef
    .required(false)
    .repeatable(true)
    .argument("value", true)
    .callback(Poco::Util::OptionCallback<PocoObserver>(&observer, &PocoObserver::handleExtraArgs));
    options->addOption(extra_args_ef);

    Poco::Util::Option extra_args_es("es", "", "string arg");
    extra_args_es
    .required(false)
    .repeatable(true)
    .argument("value", true)
    .callback(Poco::Util::OptionCallback<PocoObserver>(&observer, &PocoObserver::handleExtraArgs));
    options->addOption(extra_args_es);

    Poco::Util::Option extra_args_ez("ez", "", "boolean arg");
    extra_args_ez
    .required(false)
    .repeatable(true)
    .argument("value", true)
    .callback(Poco::Util::OptionCallback<PocoObserver>(&observer, &PocoObserver::handleExtraArgs));
    options->addOption(extra_args_ez);
}

- (void)addTestWithKey:(std::string)key Value:(std::string)value {
    NSString *testList = [NSString stringWithUTF8String:value.c_str()];
    [self.testIds addObjectsFromArray:[testList componentsSeparatedByString:@","]];
}

- (void)handleHelpWithKey:(std::string)key Value:(std::string)value {
    helpNeeded = true;

    NSString *callLine = @"app_ios";
    for (Poco::Util::OptionSet::Iterator it = optionSet.begin(); it != optionSet.end(); ++it) {
        NSString *optionString = [NSString stringWithUTF8String:it->fullName().c_str()];
        if(!it->required()) {
            optionString = [NSString stringWithFormat:@"[%@]", optionString];
        }

        callLine = [callLine stringByAppendingString:[NSString  stringWithFormat:@" %@", optionString]];
    }
    NSLog(@"%@", callLine);
    NSLog(@"");
    NSLog(@"Options:");

    for (Poco::Util::OptionSet::Iterator it = optionSet.begin(); it != optionSet.end(); ++it) {
        NSString *fullname = [NSString stringWithUTF8String:it->fullName().c_str()];
        NSString *shortName = [NSString stringWithUTF8String:it->shortName().c_str()];
        shortName = [shortName isEqualToString:@""] ? @"" : [NSString stringWithFormat:@", -%@", shortName];
        NSString *desc = [NSString stringWithUTF8String:it->description().c_str()];
        NSString *parameter = it->takesArgument() ? [NSString stringWithFormat:@"<%@>", [NSString stringWithUTF8String:it->argumentName().c_str()]] : @"";

        NSString *optionLine = [NSString stringWithFormat:@"\t-%@%@ %@", fullname, shortName, parameter];
        for(NSUInteger i = optionLine.length; i < 30; ++i) {
            optionLine = [optionLine stringByAppendingString:@" "];
        }
        NSLog(@"%@%@", optionLine, desc);
    }
}

- (void)handleVerboseWithKey:(std::string)key Value:(std::string)value {
}

- (void)handleExtraArgsWithKey:(std::string)name Value:(std::string)value {

     NSArray *valueList = [[NSString stringWithUTF8String:value.c_str()] componentsSeparatedByString:@"="];

    if (valueList.count != 2)
    {
        NSLog(@"Parameter parsing error: \"%s\" is wrong please use valid key=value pair", value.c_str());
        throw Poco::Exception("Parameter parsing error: \"" + value + "\" is wrong please use valid key=value pair");
    }
    else
    {
        NSString *sub_key = valueList[0];
        NSString *sub_value = valueList[1];

        //gfx specific "-" syntax;
        if ([sub_key hasPrefix:@"-"])
            sub_key = [sub_key stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"-"]];

        if (name == "ei")
        {
            self.rawn[sub_key] = [NSNumber numberWithInteger:[sub_value integerValue]];
        }
        else if (name == "ef")
        {
            self.rawn[sub_key] = [NSNumber numberWithFloat:[sub_value floatValue]];
        }
        else if (name == "es")
        {
            self.raws[sub_key] = sub_value;
        }
        else if (name == "ez")
        {
            if ([sub_value isEqualToString:@"true"] || [sub_value isEqualToString:@"1"]) {
                self.rawz[sub_key] = [NSNumber numberWithBool:true];
            }
            else if ([sub_value isEqualToString:@"false"] || [sub_value isEqualToString:@"0"]) {
                self.rawz[sub_key] = [NSNumber numberWithBool:false];
            }
        }
    }
}


- (void)parse {
    observer.delegate = self;
    self.testIds = [[NSMutableArray alloc] init];
    self.rawn = [[NSMutableDictionary alloc] init];
    self.raws = [[NSMutableDictionary alloc] init];
    self.rawz = [[NSMutableDictionary alloc] init];
    self.config = [[NSMutableDictionary alloc] init];

    [self fillOptionSet:&optionSet];

    NSArray *arguments = [[NSProcessInfo processInfo] arguments];

    for (Poco::Util::OptionSet::Iterator it = optionSet.begin(); it != optionSet.end(); ++it) {
        for (int index = 0; index < arguments.count; ++index) {
            NSString *shortName = [NSString stringWithFormat:@"-%@", [NSString stringWithUTF8String:it->shortName().c_str()]];
            NSString *fullname = [NSString stringWithFormat:@"-%@", [NSString stringWithUTF8String:it->fullName().c_str()]];
            NSString *name = [NSString stringWithUTF8String:it->fullName().c_str()];
            if([arguments[index] isEqualToString:shortName] || [arguments[index] isEqualToString:fullname]) {
                NSLog(@"CommandLineParser found option: %@", name);

                NSString *argument = @"";

                if (it->takesArgument()) {
                    NSString *possibleArgument = @"";
                    if(arguments.count > index+1) {
                        possibleArgument = arguments[index+1];
                        if([possibleArgument hasPrefix:@"-"]) {
                           possibleArgument = @"";
                        }
                    }

                    if([possibleArgument isEqualToString:@""] && it->argumentRequired()) {
                        NSLog(@"Required argument is missing for option: %@", name);
                        throw Poco::Exception("Commandline parsing error.");
                    }

                    argument = possibleArgument;
                }

                if(!it->repeatable() && [self.config objectForKey:@"name"] != nil) {
                    NSLog(@"Non-Repeatable option repeated: %@", name);
                    throw Poco::Exception("Commandline parsing error.");
                }

                if(it->callback() != NULL) {
                    it->callback()->invoke([name UTF8String], [argument UTF8String]);
                }

                [self.config setObject:argument forKey:[NSString stringWithUTF8String:it->binding().c_str()]];

            } else if(it->required()) {
                NSLog(@"Required option is missing: %@", name);
                throw Poco::Exception("Commandline parsing error.");
            }
        }

        if(helpNeeded) {
            exit(1);
        }
    }

    [self prepareDescriptors];
}


- (void) prepareDescriptors {
    TFWDescriptors.descriptors().clear();

    // Create the descriptions from the json data.
    NSString* descPath = [self getDescPath];
    NSArray *DescJsons = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:descPath error:nil];

    for (NSString *testId in self.testIds)
    {
        NSString *s = [testId stringByAppendingString:@".json"];
        if([DescJsons containsObject:s])
        {
            @try
            {
                tfw::Descriptor desc;
                ng::JsonValue config;
                ng::Result result;
                NSString *path = [descPath stringByAppendingString:s];
                config.fromFile([path UTF8String], result);

                config = [self updateConfig:config];

                std::string updatedConfig;
                config.toString(updatedConfig);
                tfw::Descriptor::fromJsonString(updatedConfig, &desc);

                TFWDescriptors.descriptors().push_back(desc);
            }
            @catch (...)
            {
                NSLog(@"Description creation failed.");
            }
        }
        else
        {
            NSLog(@"Missing test json: %@", s);
        }
    }

    if(TFWDescriptors.descriptors().empty())
        throw Poco::Exception("Descriptors are missing.");
}


- (ng::JsonValue) updateConfig:(ng::JsonValue)c {
    if([self.config objectForKey:@"width"] != nil || [self.config objectForKey:@"height"] != nil)
    {
        CGFloat scale = 1;
        if ([[UIScreen mainScreen] respondsToSelector:@selector(nativeScale)])
        {
            scale = [[UIScreen mainScreen] nativeScale];
        }
        else if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
        {
            scale = [[UIScreen mainScreen] scale];
        }
        float width = [[UIScreen mainScreen] bounds].size.width * scale;
        float height = [[UIScreen mainScreen] bounds].size.height * scale;

        if([self.config objectForKey:@"width"] != nil)
            width = [[self.config objectForKey:@"width"] floatValue];
        if([self.config objectForKey:@"height"] != nil)
            height = [[self.config objectForKey:@"height"] floatValue];

        c["raw_config"]["virtual_resolution"] = true;
        c["raw_config"]["test_width"] = width;
        c["raw_config"]["test_height"] = height;
    }


    for (NSString *key in [self.rawn allKeys]) {
        c["raw_config"][[key UTF8String]] = [self.rawn[key] floatValue];
    }

    for (NSString *key in [self.raws allKeys]) {
        c["raw_config"][[key UTF8String]] = [self.raws[key] UTF8String];
    }

    for (NSString *key in [self.rawz allKeys]) {
        c["raw_config"][[key UTF8String]] = [self.rawz[key] boolValue];
    }

    return c;
}


- (tfw::DescriptorList)getDescriptors {
    return TFWDescriptors;
}


- (NSString *)getInputPath
{
    NSString *path = [[NSBundle mainBundle] resourcePath];
    path = [path stringByAppendingString:@"/"];

    if(self.config[@"base_path"] != nil) {
        path = [path stringByAppendingString:self.config[@"base_path"]];
        path = [path stringByAppendingString:@"/"];
    }

    return path;
}


- (NSString *)getDescPath
{
    NSString *path = [self getInputPath];

    path = [path stringByAppendingString:@"config"];
    path = [path stringByAppendingString:@"/"];

    return path;
}


- (NSString *)getOutputPathWithDefault:(NSString *)default_path {
    NSString* path = [[NSFileManager defaultManager] applicationSupportDirectory];
    path = [path stringByAppendingString:@"/"];

    if(self.config[@"base_path"] != nil) {
        path = [path stringByAppendingString:self.config[@"base_path"]];
        path = [path stringByAppendingString:@"/"];
    }

    if(self.config[@"resultdir"] != nil) {
        path = [path stringByAppendingString:self.config[@"resultdir"]];
        path = [path stringByAppendingString:@"/"];

    } else {
        path = [path stringByAppendingString:@"results"];
        path = [path stringByAppendingString:@"/"];
    }

    return path;
}

@end



void PocoObserver::addTest(const std::string &key, const std::string &value) {
    [delegate addTestWithKey:(std::string)key Value:(std::string)value];
}

void PocoObserver::handleHelp(const std::string &key, const std::string &value) {
    [delegate handleHelpWithKey:(std::string)key Value:(std::string)value];
}

void PocoObserver::handleVerbose(const std::string &key, const std::string &value) {
    [delegate handleVerboseWithKey:(std::string)key Value:(std::string)value];
}

void PocoObserver::handleExtraArgs(const std::string &key, const std::string &value) {
    [delegate handleExtraArgsWithKey:(std::string)key Value:(std::string)value];
}
