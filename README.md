# CGSSAssetsDownloader
A download tool for CGSS (IDOL MASTER CINDERELLA GIRLS STARLIGHT STAGE).

[Download release](https://github.com/toyobayashi/CGSSAssetsDownloader/releases)

## Usage

``` Bash
$ CGSSAssetsDownloader <-v resource_version> [-o option or filename] [-u]
```
If you don't know the <resource_version>, try to visit https://starlight.kirara.ca/api/v1/info 

option could be:

【bgm】All background music will be downloaded.

【live】All live music will be downloaded.

【card】All unity3d files that contain card background will be downloaded.

【icon】All unity3d files that contain 124x124 card icon will be downloaded.

【score】All bdb files that contain music score will be downloaded.

If you use -u, the file will be copied to dl\.

## Powered by

* [hcadec](https://github.com/alama/hcadec)
* [OpenCGSS/DereTore](https://github.com/OpenCGSS/DereTore)
* [SQLite](https://sqlite.org/)
* [UnityLz4](https://github.com/subdiox/UnityLz4)
<!--* [wget](https://www.gnu.org/software/wget/)-->

## Copyright

The copyright of CGSS and its related content is held by [BANDAI NAMCO Entertainment, Inc.](https://bandainamcoent.co.jp/)
