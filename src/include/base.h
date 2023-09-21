#pragma once
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

template<class T,int N>
int countn(T(&)[N]) {return N;}

