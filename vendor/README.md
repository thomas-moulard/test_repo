The tar.gz file in this directory was created using the 1.6.1 tar.gz release of Poco from:

https://github.com/pocoproject/poco/releases/tag/poco-1.6.1-release

It was extracted, and the following directories were deleted to save on size:

- ApacheConnector
- CppParser
- CppUnit
- Crypto
- Data
- JSON
- MongoDB
- Net
- NetSSL_OpenSSL
- NetSSL_Win
- PDF
- PageCompiler
- PocoDoc
- ProGen
- SevenZip
- Util
- XML
- Zip

Basically only leaving small files and the Foundation source code.

It should be built with these CMake options:

```
-DENABLE_XML:BOOL=OFF
-DENABLE_JSON:BOOL=OFF
-DENABLE_MONGODB:BOOL=OFF
-DENABLE_UTIL:BOOL=OFF
-DENABLE_NET:BOOL=OFF
-DENABLE_NETSSL:BOOL=OFF
-DENABLE_CRYPTO:BOOL=OFF
-DENABLE_DATA:BOOL=OFF
-DENABLE_ZIP:BOOL=OFF
-DENABLE_PAGECOMPILER:BOOL=OFF
-DENABLE_PAGECOMPILER_FILE2PAGE:BOOL=OFF
```

In order to disable the parts which were removed.
