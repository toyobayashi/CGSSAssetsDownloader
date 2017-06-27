# CGSSAssetsDownloader
欢迎使用CGSSAssetsDownloader。

下载 [CGSSAssetsDownloader](https://github.com/MoebutagataShiozakana/CGSSAssetsDownloader/releases)

## 更新历史
v1.1.1
1. 代码改造，功能上和1.1没有区别。

v1.1
1. 从CGSSMusicDownloader更名为CGSSAssetsDownloader，新增下载卡片背景unity3d功能
2. 优化下载逻辑，修复禁止访问的问题

v1.0
1. 由于游戏服务器设置了访问权限，已失效

## 说明
本工具由C++编写，用于下载《偶像大师灰姑娘女孩星光舞台》的所有音乐（包括背景音乐和live乐曲，不包括角色语音）和卡片背景资源。本工具借鉴了前代starlight_downloader，现在由于游戏的服务器结构更变导致starlight_downloader不能再继续使用而starlight_downloader的作者也没再更新工具。所以本人尝试写了这么个东西。

原理：先从游戏的服务器下载被压缩编码的资源文件，再通过解压或解码获得。

## 用法
在此工具根目录下运行cmd命令行控制台，执行命令
```Bash
CGSSAssetsDownloader [版本号] [选项]
```
[选项]可以是bgm, live, card。

若缺省选项参数，则只会下载对应版本的资源表。

如果不知道版本号，可以访问 https://starlight.kirara.ca/api/v1/info 查询。

## 提醒
1. 第一次下载根据网络状况可能需要30-90分钟，请不要关闭命令行窗口，耐心等候。根据实际的网络状况，也可能会出现中途卡住没有下载速度的情况，这时可以ctrl+c结束，尝试重新下载，可以断点续传。

2. 背景音乐会保存在bgm\目录下，live乐曲会保存在live\目录下，卡片unity3d文件会保存在card\目录下，资源表数据库文件会保存在data\目录下，更新历史会保存在log\目录下。

3. 如果游戏数据库有版本更新，此工具会自动下载数据库并根据目录内已存在的资源进行资源更新，请不要移动或删除已下载好的文件，否则会重头开始下载。

4. 本工具不支持解包unity3d资源。

5. 免费使用，禁止用于商业用途。

## 借用的外部工具
wget

HCADecoder

DeretoreACB

SSDecompress

