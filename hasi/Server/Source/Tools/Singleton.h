#ifndef IVDA_SINGLETON_H
#define IVDA_SINGLETON_H

#define SINGLETON_IVDA_STACK(ClassName)\
 public:\
 static ClassName& Instance()\
  {\
    static ClassName instance;\
    return instance;\
  }\
 private:\
 ClassName();\
 ClassName(const ClassName&);

// ATTENTION: instance variable will not be deleted thus it is guaranteed that instance still lives when stack objects are being destroyed (e.g. to track killed threads)
//#define IVDA_NEVER_USE_HEAP_SINGLETON
#ifdef IVDA_NEVER_USE_HEAP_SINGLETON
#define SINGLETON_IVDA_HEAP(ClassName) SINGLETON_IVDA_STACK(ClassName)
#else
#define SINGLETON_IVDA_HEAP(ClassName)\
 public:\
 static ClassName& Instance()\
  {\
    static ClassName* instance = new ClassName();\
    return *instance;\
  }\
 private:\
 ClassName();\
 ClassName(const ClassName&);
#endif

#endif // IVDA_SINGLETON_H