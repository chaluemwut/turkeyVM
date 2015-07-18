#ifndef H_NATIVE
#define H_NATIVE


typedef struct
{
    char* method_name;
    char* desc;
    void (*action)();
}Binding;




extern Binding nativeMethods[];


extern void getName0();
extern void getClass();
extern Object* getClass_name(char* classname);






#endif
