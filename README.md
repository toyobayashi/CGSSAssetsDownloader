# CGSSAssetsDownloader
欢迎使用CGSSAssetsDownloader。

下载 [CGSSAssetsDownloader](https://github.com/MoebutagataShiozakana/CGSSAssetsDownloader/releases)

## 说明
本工具由C++编写，用于下载《偶像大师灰姑娘女孩星光舞台》的游戏资源。本工具借鉴了前代starlight_downloader，现在由于游戏的服务器结构更变导致starlight_downloader不能再继续使用而starlight_downloader的作者也没再更新工具。所以我尝试写了这么个东西。

原理：先从游戏的服务器下载被压缩编码的资源文件，再通过解压或解码获得。

## 用法
在此工具根目录下运行cmd命令行控制台，执行命令
```Bash
CGSSAssetsDownloader [版本号] [选项]
```
[选项]可以是：

【bgm】下载所有背景音乐并自动解码

【live】下载所有live乐曲并自动解码

【card】下载所有包含卡片背景的unity3d文件

【icon】下载所有包含卡片图标的unity3d文件

【score】下载所有包含谱面的bdb文件

也可以是需要下载的文件的文件名。

若缺省选项参数，则只会下载对应版本的资源表。

如果不知道版本号，可以访问 https://starlight.kirara.ca/api/v1/info 查询。

## 提醒
1. 第一次下载根据网络状况的不同需要不同的耗时，请不要关闭命令行窗口，耐心等候。根据实际的网络状况，也可能会出现中途卡住没有下载速度的情况，这时可以ctrl+c结束，尝试重新执行命令，可以断点续传。

2. 背景音乐会保存在bgm\目录下，live乐曲会保存在live\目录下，卡片背景unity3d文件会保存在card\目录下，卡片图标unity3d文件会保存在icon\目录下，谱面bdb文件会保存在score\目录下，资源表数据库文件会保存在data\目录下，更新历史会保存在log\目录下，单文件下载会保存在dl\目录下。

3. 如果游戏数据库有版本更新，此工具会自动下载数据库并根据目录内已存在的资源进行资源更新，请不要移动或删除已下载好的文件，否则会重头开始下载。

4. 本工具不支持解包unity3d资源。

5. 免费使用，禁止用于商业用途。

## 借用的外部工具
wget

HCADecoder

DeretoreACB

SSDecompress

