﻿# Http web sever
## HUST COMPUTER NETWORKING LAB1
### by zbl
### 1.项目描述
    这是计算机网络第一个实验。要求我们实现一个web sever。
    本项目实现了一个非常简单的基于window socket的多线程web sever。
    该项目仅包含level1server.cpp一个文件。
### 2.运行环境
    本项目在window10 VS2017下可以成功运行。
    打开level1server.cpp并编译运行即可。
### 3.项目特性
    1.支持web sever的基本功能。能够监听、解析各种http报文的方法、请求文件目录、请求文件类型。能够向浏览器返回一个标准的http报文，包含浏览器请求的html、css、jpg、png等各种格式的文件。
    2.支持自定义设置sever的工作目录、ip、port。默认获取程序当前运行目录的作为工作目录，127.0.0.1作为ip，80作为port。
    3.支持多线程并发处理。
    4.能够在文件不存在时返回自定义的404页面。
    5.能够在控制台返回运行状态的信息，包括来自外部的http请求使用的方法、请求文件目录等。