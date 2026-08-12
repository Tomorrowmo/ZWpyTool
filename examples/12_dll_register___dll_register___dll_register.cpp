/*
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

#include "stdafx.h"
#include "VxApi.h"

enum ArgType
{
    eatKey = 0, //键名
    eatName,    //插件名
    eatDes,     //描述
    eatLoc,     //位置
    eatRes      //资源
};

int get_args(int argc, char *argv[], char args[][256]);
int register_plugin(char args[][256]);

int main
(
int argc,
char *argv[]
)
{
    char args[5][256] = {0};
    if (get_args(argc, argv, args))
        return 0;
    printf("注册 %s...\n", args[eatName]);
    int nRst = register_plugin(args);
    printf("注册 %s!\n", nRst ? "失败" : "成功");
    return 0;
}

void help()
{
    printf("*********************************** DllRegister ***********************************\n");
    printf("描述\n");
    printf("这是一个将dll注册到ZW3D的示例。\n\n");
    printf("语法\n");
    printf("DllRegister [选项] [值]\n\n");
    printf("键\n");
    printf("-n  键名\n");
    printf("-a  插件名\n");
    printf("-d  插件的描述\n");
    printf("-l  插件的位置\n");
    printf("-r  [可选] 插件的资源文件\n");
    printf("-h  帮助\n\n");
    printf("示例\n");
    printf("DllRegister -n Pattern -a Pattern.dll -d \"这是一个示例！\" -l C:\\code\\Pattern\\Debug -r "
           "C:\\code\\Pattern\\Debug\\Pattern.zrc\n");
    printf("结果是：\n");
    printf("注册 Pattern.dll...\n");
    printf("注册成功！\n");
    printf("***********************************************************************************\n");
}

int get_args
(
int argc,
char *argv[],
char args[][256]
)
{
    int nRst = 0;
    if (argc < 2)
    {
        help();
        return 1;
    }
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h"))
        {
            help();
            return 1;
        }
        if (!strcmp(argv[i], "-n"))
        {
            i++;
            strcpy(args[eatKey], argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-a"))
        {
            i++;
            strcpy(args[eatName], argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-d"))
        {
            i++;
            strcpy(args[eatDes], argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-l"))
        {
            i++;
            strcpy(args[eatLoc], argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-r"))
        {
            i++;
            strcpy(args[eatRes], argv[i]);
            continue;
        }
    }
    if (!args[eatKey] || !args[eatName] || !args[eatLoc])
        return 1;
    return 0;
}

int register_plugin
(
char args[][256]
)
{
    vxPath *pPath = nullptr;
    int nCnt = 0;
    int is64 = 0;
    int nRst = 0;
#ifdef _WIN64
    is64 = 1;
#endif
    if (cvxPluginRegPathGet(&pPath, &nCnt, is64))
    {
        nRst = 1;
        goto error_exit;
    }
    //将当前dll信息写入注册表，包括'Application'，'Description'，'Load'，'Location'和'Resource'。
    char szPath[MAX_PATH] = "\0";
    HKEY hKey;
    DWORD dwDisposition = 0;
    for (int i = 0; i < nCnt; i++)
    {
        sprintf(szPath, "%s\\%s", pPath[i], args[eatKey]);
        if (RegCreateKeyExA(HKEY_CURRENT_USER, szPath, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,
                            &dwDisposition) != ERROR_SUCCESS)
        {
            nRst = 1;
            goto error_exit;
        }
        if (RegSetValueExA(hKey, "Application", NULL, REG_SZ, (BYTE *)args[eatName], (DWORD)strlen(args[eatName])) !=
            ERROR_SUCCESS)
        {
            nRst = 1;
            goto error_exit;
        }
        if (RegSetValueExA(hKey, "Description", NULL, REG_SZ, (BYTE *)args[eatDes], (DWORD)strlen(args[eatDes])) !=
            ERROR_SUCCESS)
        {
            nRst = 1;
            goto error_exit;
        }
        char szVal[256] = "7";
        if (RegSetValueExA(hKey, "Load", NULL, REG_SZ, (BYTE *)szVal, (DWORD)strlen(szVal)) != ERROR_SUCCESS)
        {
            nRst = 1;
            goto error_exit;
        }
        memset(szVal, 0x00, sizeof(szVal));
        _fullpath(szVal, args[eatLoc], sizeof(szVal));
        if (_access(szVal, 0))
        {
            nRst = 1;
            goto error_exit;
        }
        if (RegSetValueExA(hKey, "Location", NULL, REG_SZ, (BYTE *)szVal, (DWORD)strlen(szVal)) != ERROR_SUCCESS)
        {
            nRst = 1;
            goto error_exit;
        }
        memset(szVal, 0x00, sizeof(szVal));
        if (args[eatRes][0])
            _fullpath(szVal, args[eatRes], sizeof(szVal));
        if (RegSetValueExA(hKey, "Resource", NULL, REG_SZ, (BYTE *)szVal, (DWORD)strlen(szVal)) != ERROR_SUCCESS)
        {
            nRst = 1;
            goto error_exit;
        }
        memset(szPath, 0x00, sizeof(szPath));
    }
error_exit:
    cvxMemFree((void **)&pPath);
    return nRst;
}