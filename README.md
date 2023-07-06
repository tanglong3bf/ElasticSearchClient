# ElasticSearchClient

ElasticSearchClient is a [Drogon](https://github.com/drogonframework/drogon.git) plugin that can interact with the ElasticSearch server in a more concise way.

# Requirements

- ElasticSearch v6.8 (Only this version has been tested)

# HowToUse

## Install

For the default drogon project, you need create a new subdirectory in the `plugins` directory of the project.

Like this:

```shell
$ tree .
.
├── CMakeLists.txt
├── config.json
├── main.cc
└── plugins
    └── tl_elasticsearch
```

And then, copy all files in the `src` directory to the new subdirectory.

Like this:

```shell
$ tree .
.
├── CMakeLists.txt
├── config.json
├── main.cc
└── plugins
    └── tl_elasticsearch
        ├── ElasticSearchClient.cc
        ├── ElasticSearchClient.h
        └── ...
```

Finally, modify the `CMakeLists.txt` file of the drogon project so that this plugin can be compiled into the project.

```cmake
# aux_source_directory(plugins PLUGIN_SRC)
file(GLOB_RECURSE PLUGIN_SRC LIST_DIRECTORIES true "plugins/*.cc")
```

## config

```json
"plugins": [
    {
        "name": "tl::elasticsearch::ElasticSearchClient",
        "config": {
            // default value: localhost
            "host": "localhost",
            // default value: 9200
            "port": 9200
        }
    }
]
```

# examples

## synchronous

```cpp
// using namespace std;
// using namespace drogon;
// using namespace tl::elasticsearch;

auto esPlugin = app().getPlugin<ElasticSearchClient>();
CreateIndexParam param(3, 2);
param.addProperty(Property("title", TEXT, "ik_smart"))
    .addProperty(Property("content", TEXT, "ik_smart"))
    .addProperty(Property("views", INTEGER))
    .addProperty(Property("author")
        .addSubProperty(Property("firstName", KEYWORD))
        .addSubProperty(Property("lastName", KEYWORD)));
try {
    auto indices = esPlugin->indices();             // IndicesClientPtr
    auto response = indices->create("blog", param); // CreateIndexResponsePtr
    string result = "index created successfully, the name of index:<br>";
    result += response->getIndex();
    LOG_INFO << result;
} catch (ElasticSearchException e) {
    string result = "failed to create index, the reason of failure:<br>";
    result += e.what();
    LOG_ERROR << result;
}
```

## asynchronous

```cpp
// using namespace std;
// using namespace drogon;
// using namespace tl::elasticsearch;

auto esPlugin = app().getPlugin<ElasticSearchClient>();
CreateIndexParam param(3, 2);
param.addProperty(Property("title", TEXT, "ik_smart"))
    .addProperty(Property("content", TEXT, "ik_smart"))
    .addProperty(Property("views", INTEGER))
    .addProperty(Property("author")
        .addSubProperty(Property("firstName", KEYWORD))
        .addSubProperty(Property("lastName", KEYWORD)));

auto indices = esPlugin->indices();               // IndicesClientPtr
indices->create("blog", [](CreateIndexResponsePtr &response) {
    string result = "index created successfully, the name of index:<br>";
    result += response->getIndex();
    LOG_INFO << result;
}, [](ElasticSearchException &&exception) {
    string result = "failed to create index, the reason of failure:<br>";
    result += exception.what();
    LOG_ERROR << result;
}, param);
```
