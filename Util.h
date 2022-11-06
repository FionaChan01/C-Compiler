//
// Created by FiFi on 2022/5/24.
//

#ifndef COMPILER_UTIL_H
#define COMPILER_UTIL_H
#include "iostream"
#include "string"




string toLowerCase(string str) {
    for (char & i : str) {
        if (i >= 'A' && i <= 'Z') {
            i = i - 'A' + 'a';
        }
    }
    return str;
}

string fillString(string in, int begin, int end)
{
    char ans[end - begin +3];
    fill(ans,ans+end - begin +3,0);
    int i=begin;
    while(i<end){
        ans[i - begin] = in[i];
        i++;
    }
    return ans;
}

string addSlash(string s) {
    string ans = "";
    int i=0;
    int size = s.size();
    while(i<size){
        if (s[i] == '\\') {
            ans += "\\\\";
        }
        else {
            ans += s[i];
        }
        ++i;
    }
    return ans;
}

#endif //COMPILER_UTIL_H
