#include <compose/SVGDocumentCache.h>
#include <memory>
#include <unordered_map>

namespace Compose
{
  namespace
  {
    std::unordered_map<std::string, std::unique_ptr<lunasvg::Document>> &pathDocuments()
    {
      static std::unordered_map<std::string, std::unique_ptr<lunasvg::Document>> documents;
      return documents;
    }

    std::unordered_map<std::string, std::unique_ptr<lunasvg::Document>> &contentDocuments()
    {
      static std::unordered_map<std::string, std::unique_ptr<lunasvg::Document>> documents;
      return documents;
    }

    const lunasvg::Document *insertDocument(std::unordered_map<std::string, std::unique_ptr<lunasvg::Document>> &cache,
                                            const std::string &key, std::unique_ptr<lunasvg::Document> document)
    {
      if(!document)
        return nullptr;

      const auto *result = document.get();
      cache.emplace(key, std::move(document));
      return result;
    }
  }

  const lunasvg::Document *SVGDocumentCache::documentFor(const SVGPath &path)
  {
    const auto key = path.it.generic_string();
    auto &cache = pathDocuments();
    if(const auto found = cache.find(key); found != cache.end())
      return found->second.get();

    return insertDocument(cache, key, lunasvg::Document::loadFromFile(path.it.string()));
  }

  const lunasvg::Document *SVGDocumentCache::documentFor(const SVGFileContent &content)
  {
    const auto &key = content.content;
    auto &cache = contentDocuments();
    if(const auto found = cache.find(key); found != cache.end())
      return found->second.get();

    return insertDocument(cache, key, lunasvg::Document::loadFromData(content.content));
  }
}
