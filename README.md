# ElasticSearchClient

ElasticSearchClient is a [Drogon](https://github.com/drogonframework/drogon.git) plugin that can interact with the ElasticSearch server in a more concise way.

# HowToUse

## Install

For the default drogon project, you only need to copy two files to the `plugins` directory of the project to use them.

Like this:

```shell
$ tree .
.
├── CMakeLists.txt
├── config.json
├── main.cc
└── plugins
    ├── ElasticSearchClient.cc
    └── ElasticSearchClient.h
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
CreateIndexParam param;
param.addProperty(new Property("title", TEXT, "ik_smart"))
    .addProperty(new Property("content", TEXT, "ik_smart"))
    .addProperty(new Property("views", INTEGER));

auto indices = esPlugin->indices();             // IndicesClientPtr
auto response = indices->create("blog", param); // CreateIndexResponsePtr
if (response->isSuccess()) {
    string result = "index created successfully, the name of index:<br>";
    result += dynamic_pointer_cast<CreateIndexSuccessResponse>(response)->getIndex();
    LOG_INFO << result;
} else {
    string result = "failed to create index, the reason of failure:<br>";
    result += dynamic_pointer_cast<CreateIndexFailedResponse>(response)->getError()->getReason();
    LOG_INFO << result;
}
```

## asynchronous

```cpp
// using namespace std;
// using namespace drogon;
// using namespace tl::elasticsearch;

auto esPlugin = app().getPlugin<ElasticSearchClient>();
CreateIndexParam param;
param.addProperty(new Property("title", TEXT, "ik_smart"))
    .addProperty(new Property("content", TEXT, "ik_smart"))
    .addProperty(new Property("views", INTEGER));

auto indices = esPlugin->indices();               // IndicesClientPtr
indices->create("blog", [](CreateIndexResponsePtr &response) {
    if (response->isSuccess()) {
        string result = "index created successfully, the name of index:<br>";
        result += dynamic_pointer_cast<CreateIndexSuccessResponse>(response)->getIndex();
        LOG_INFO << result;
    } else {
        string result = "failed to create index, the reason of failure:<br>";
        result += dynamic_pointer_cast<CreateIndexFailedResponse>(response)->getError()->getReason();
        LOG_INFO << result;
    }
}, [](ElasticSearchException &&exception) {
    LOG_ERROR << exception.what();
}, param);
```
