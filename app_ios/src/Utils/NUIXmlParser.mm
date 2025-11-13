/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "NUIXmlParser.h"
#import "Utils/NUIUtilities.h"

@implementation NUIXmlParser
{
    NSXMLParser *parser;
}

- (id)init
{
    return self;
}


- (NSMutableDictionary *)parseXml:(NSData *)data
{
    self.netResults = [[NSMutableDictionary alloc] init];
    self.netMaximum = [[NSMutableDictionary alloc] init];
    self.netDevices = [[NSMutableArray alloc] init];
    self.netTests = [[NSMutableArray alloc] init];
    self.errorParsing = FALSE;


    NSString *strXML = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    strXML = [strXML stringByReplacingOccurrencesOfString:@"" withString:@""];
    NSRange range = [strXML rangeOfString:@"<?xml"];
    if(range.location != NSNotFound)
    {
        strXML = [[strXML substringFromIndex:NSMaxRange(range)-5] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
        NSData *xmldata = [strXML dataUsingEncoding:NSUTF8StringEncoding allowLossyConversion:YES];

        parser = [[NSXMLParser alloc] initWithData:xmldata];
        [parser setDelegate:self];

        [parser setShouldProcessNamespaces:NO];
        [parser setShouldReportNamespacePrefixes:NO];
        [parser setShouldResolveExternalEntities:NO];

        [parser parse];
    }

    return [NSMutableDictionary dictionaryWithDictionary:@{ @"results" : self.netResults, @"devices" : self.netDevices, @"maximum" : self.netMaximum}];
}


- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError {

    NSString *errorString = [NSString stringWithFormat:@"Error code %li", (long)[parseError code]];
    NSLog(@"Error parsing XML: %@", errorString);

    self.errorParsing=YES;
}


- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict{
    if ([elementName isEqualToString:@"test"])
    {
        self.test = [NSMutableDictionary dictionaryWithDictionary:attributeDict];
        NSMutableDictionary* maxdict = [[NSMutableDictionary alloc] init];
        [maxdict setObject:@([attributeDict[@"lower-better"] boolValue]) forKey:@"lower-better"];
        [maxdict setObject:attributeDict[@"metric"] forKey:@"metric"];

        NSString *postfix = attributeDict[@"subresultid"] == nil ? @"0" : attributeDict[@"subresultid"];
        [self.netMaximum setObject:maxdict forKey:[NSString stringWithFormat:@"%@_%@", attributeDict[@"id"], postfix] ];
    }
    else if ([elementName isEqualToString:@"device"])
    {
        self.currentDevice = attributeDict[@"name"];

        self.deviceDict = [NSMutableDictionary dictionaryWithDictionary:attributeDict];
        [self.deviceDict setObject:[[NSMutableDictionary alloc] init] forKey:@"results"];
    }
    else if ([elementName isEqualToString:@"result"])
    {
        NSNumber *val = [NSNumber numberWithDouble:[attributeDict[@"value"] doubleValue]];
        val = [NUIUtilities ApplyMetric:self.netMaximum[attributeDict[@"test"]][@"metric"] To:[val doubleValue]];

        NSString *postfix = attributeDict[@"subresultid"] == nil ? @"0" : attributeDict[@"subresultid"];
        NSString *key = [NSString stringWithFormat:@"%@_%@", attributeDict[@"test"], postfix];
        [self.deviceDict[@"results"] setObject:val forKey:key];

        NSMutableDictionary* actualMax = self.netMaximum[key];

        if([actualMax objectForKey:@"value"] == nil)
        {
            [actualMax setObject:val forKey:@"value"];
        }
        else
        {
            if([actualMax[@"lower-better"] boolValue])
            {
                if([actualMax[@"value"] doubleValue] > [val doubleValue])
                    [actualMax setObject:@([val doubleValue]) forKey:@"value"];
            }
            else
            {
                if([actualMax[@"value"] doubleValue] < [val doubleValue])
                    [actualMax setObject:@([val doubleValue]) forKey:@"value"];
            }
        }

        self.result = [NSMutableDictionary dictionaryWithDictionary:attributeDict];
        [self.result setObject:self.currentDevice forKey:@"device"];
        [self.result setObject:self.deviceDict[@"image"] forKey:@"image"];
        [self.result setObject:[NSNumber numberWithDouble:[val doubleValue]] forKey:@"value"];
    }
}


- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName{
    if ([elementName isEqualToString:@"test"])
    {
        [self.netTests addObject:[self.test copy]];
        NSString *postfix = self.test[@"subresultid"] == nil ? @"0" : self.test[@"subresultid"];
        [self.netResults setObject:[[NSMutableArray alloc] init]
                            forKey:[NSString stringWithFormat:@"%@_%@", self.test[@"id"], postfix] ];
    }
    else if ([elementName isEqualToString:@"device"])
    {
        self.currentDevice = nil;

        [self.netDevices addObject:self.deviceDict];
        self.deviceDict = nil;
    }
    else if ([elementName isEqualToString:@"result"])
    {
        NSString *postfix = self.result[@"subresultid"] == nil ? @"0" : self.result[@"subresultid"];
        self.result[@"test"] = [NSString stringWithFormat:@"%@_%@", self.result[@"test"], postfix];
        [self.netResults[self.result[@"test"]] addObject:[self.result copy]];
    }
}

- (void)parserDidEndDocument:(NSXMLParser *)parser {

    if (self.errorParsing == NO)
    {
        NSLog(@"XML processing done!");
    }
    else
    {
        NSLog(@"Error occurred during XML processing");
    }
}



@end
