#pragma once

#define STRINGIZE2(x) #x 
#define STRINGIZE(x) STRINGIZE2(x)

#ifndef RevisionSHA
#define __REVISION__		"BUILT MANUALLY"
#else
#define __REVISION__		RevisionSHA
#endif

#ifndef RevisionDate
#define __REVISION_DATE__	"NOT AVAILABLE"
#else
#define __REVISION_DATE__	RevisionDate
#endif