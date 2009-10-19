// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef _SINGLETON_H
#define _SINGLETON_H

template<class T>
class Singleton
{
        public:
		static T* getInstance()
		{
			if(instance == 0){
				instance = new T;
			}
			return instance;
		}
		
        private:
		Singleton(){}
		static T* instance;
};

#endif

