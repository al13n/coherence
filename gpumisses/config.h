#pragma once

#define CLOSEST_RANGE_INSERT_LIMIT (1<<4) 
//#define MAX_AGE_LIMIT 100
//#define MAX_FULLRANGE_LIMIT (1<<12)
#define MAX_SIZE_DIRECTORY 256 /*kilobytes*/
#define SIZE_OF_COMPARTMENT (1<<12)
#define COUNT_LINK_SIZE_COMPARTMENT 0
#define SIZE_OF_PATTERN (1<<4)
#define IS_ACCURATE_ADDRESS_EXIST true 
#define IS_ACCURATE_ADDRESS_DIRTY true
#define MAX_PERCENT_REDUCE_BEFORE_EVICT 0 
#define GREATER_THAN_PERCENT_PATTERNS 0.001
#define GET_PATTERN_ON_UPDATE 0 
#define GET_PATTERN_ON_QUERY 0
#define GET_PATTERN_ON_CREATION 1
#define GET_PATTERN_ON_EVICTION 1
