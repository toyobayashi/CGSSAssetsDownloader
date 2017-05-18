# CGSSMusicDownloader
欢迎使用CGSSMusicDownloader ver1.0。

## 下载
[CGSSMusicDownloader ver1.0 Release](https://github.com/MoebutagataShiozakana/CGSSMusicDownloader/releases)

## 说明
本工具由C++编写，用于下载《偶像大师灰姑娘女孩星光舞台》的所有音乐资源（包括背景音乐和live乐曲，不包括角色语音）。

本工具借鉴了前代starlight_downloader，现在由于游戏的服务器结构更变导致starlight_downloader不能再继续使用。

而starlight_downloader的作者也没再更新工具。所以本人尝试写了这么个东西。

原理：先从游戏的服务器下载被压缩编码的音乐资源文件，再通过解码获得。

## 用法
在此目录下运行cmd命令行，输入“CGSSMusicDownloader [版本号]”回车，

或者用记事本打开download.bat文件，编辑里面的版本号，保存之后运行该.bat文件，即可开始自动下载。

如果不知道版本号，可以访问https://starlight.kirara.ca/api/v1/info

## 提醒
1.本工具【不能】下载图片资源、unity3d模型、角色语音、谱面等，【只能】下载音乐。

2.第一次下载根据网络状况可能需要30-90分钟，请不要关闭命令行窗口，耐心等候。根据实际的网络状况，也可能会出现中途卡住没有下载速度的情况，这时可以ctrl+c结束，尝试重新下载。

3.背景音乐会保存在b\目录下，live乐曲会保存在l\目录下，数据库文件和下载列表文本文件会保存在data\目录下。

4.如果游戏数据库有版本更新，此工具会自动下载数据库根据目录内的资源进行资源更新，请不要移动或删除资源文件也不要重命名任何文件夹，否则会重头开始下载所有音乐资源或者无法进行下载。

5.禁止用于商业用途。

## 借用的外部工具
wget

HCADecoder

DeretoreACB

SSDecompress

贴吧@七辻_Nyanko，微博@TTPTs
