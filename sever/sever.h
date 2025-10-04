#ifndef SEVER_H
#define SEVER_H
void SeverStart();
size_t WriteCallBack(void* content, size_t size, size_t nmemb, void* userp);
size_t WriteVectorCallback(void* content, size_t size, size_t nmemb, void* userp);
#endif // SEVER_H
