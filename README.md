# CGSSAssetsDownloader
A download tool for CGSS (IDOL MASTER CINDERELLA GIRLS STARLIGHT STAGE).

[Here to download release](https://github.com/toyobayashi/CGSSAssetsDownloader/releases)  

Emmmmmmmm... In fact I'm not good at programing with C++. So I recommend you to use [mishiro](https://github.com/toyobayashi/mishiro) which is my new __CGSS WINDOWS DESKTOP APPLICATION__ project programing with JavaScript. It can do the same thing or, even more.

## Usage

``` Bash
CGSSAssetsDownloader [-v resource_version] [-o option or filename] [-u]  
CGSSAssetsDownloader [-v resource_version] [-a] [-u] [-mp3]  
CGSSAssetsDownloader file1 file2 file3 ...
```
## Example

``` Bash
CGSSAssetsDownloader -o -bgm -u  
CGSSAssetsDownloader -v 10028005 -o gachaselect_30145.unity3d  
CGSSAssetsDownloader -v 10031250 -a -u -mp3  
CGSSAssetsDownloader path\\to\\NoSuffixFile path\\to\\ACBFile.acb path\\to\\HCAFile.hca ...
```

You can also drag no suffix file (to .unity3d), .acb file or .hca file (to .wav, or to .mp3 if you use -mp3 argument with command line) into the exe

## Arguments

[-v resource_version] [OPTIONAL] Set the resource version of game and download database.  
[-a] [OPTIONAL] Auto update bgm, live, card, icon, score assets.  
[-o bgm|live|card|icon|score|(filename)] [OPTIONAL] Read the detail below.  
[-u] [OPTIONAL] Copy files to "dl\" folder.  
[-mp3] [OPTIONAL] WAV to MP3.Default: WAV.  

-o detail:

[bgm] All background music will be downloaded.  
[live] All live music will be downloaded.  
[card] All unity3d files that contain card background will be downloaded.  
[icon] All unity3d files that contain 124x124 card icon will be downloaded.  
[score] All bdb files that contain music score will be downloaded.

## Powered by

* [HCADecoder](https://github.com/Nyagamon/HCADecoder)
* [SQLite](https://sqlite.org/)
* [UnityLz4](https://github.com/subdiox/UnityLz4)
* [curl](https://github.com/curl/curl)
* [libmp3lame](http://lame.sourceforge.net/)
* [tiny-AES-c](https://github.com/kokke/tiny-AES-c)
* [json](https://github.com/nlohmann/json)

## Copyright

The copyright of CGSS and its related content is held by [BANDAI NAMCO Entertainment Inc.](https://bandainamcoent.co.jp/)
