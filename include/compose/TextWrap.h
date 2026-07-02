#pragma once

#include <vector>
#include <functional>
#include <glibmm/ustring.h>

namespace Compose
{
  namespace text
  {
    static std::vector<Glib::ustring> splitOnChar(const Glib::ustring &text, gunichar delim)
    {
      std::vector<Glib::ustring> out;
      Glib::ustring current;
      for(const auto &ch : text)
      {
        if(ch == delim)
        {
          out.push_back(current);
          current.clear();
        }
        else
        {
          current.push_back(ch);
        }
      }
      out.push_back(current);
      return out;
    }

    static std::vector<Glib::ustring> splitOnWhitespace(const Glib::ustring &text)
    {
      std::vector<Glib::ustring> words;
      Glib::ustring current;
      for(const auto &ch : text)
      {
        if(ch == ' ' || ch == '\t')
        {
          if(!current.empty())
          {
            words.push_back(current);
            current.clear();
          }
        }
        else
        {
          current.push_back(ch);
        }
      }
      if(!current.empty())
        words.push_back(current);
      return words;
    }

    static std::vector<Glib::ustring> wrapText(
        const Glib::ustring &text, int maxWidth, const std::function<int(const Glib::ustring &)> &measure)
    {
      const Glib::ustring space = " ";
      std::vector<Glib::ustring> lines;

      auto paragraphs = splitOnChar(text, '\n');
      for(const auto &paragraph : paragraphs)
      {
        auto words = splitOnWhitespace(paragraph);
        Glib::ustring currentLine;

        for(const auto &word : words)
        {
          const auto candidate = currentLine.empty() ? word : (currentLine + space + word);
          const auto width = measure(candidate);

          if(width <= maxWidth)
          {
            currentLine = candidate;
          }
          else
          {
            if(!currentLine.empty())
              lines.push_back(currentLine);

            if(measure(word) <= maxWidth)
            {
              currentLine = word;
            }
            else
            {
              Glib::ustring partial;
              for(const auto &c : word)
              {
                auto next = partial;
                next.push_back(c);
                if(measure(next) > maxWidth && !partial.empty())
                {
                  lines.push_back(partial);
                  partial.clear();
                }
                partial.push_back(c);
              }
              currentLine = partial;
            }
          }
        }

        if(!currentLine.empty())
          lines.push_back(currentLine);

        if(words.empty())
          lines.emplace_back();
      }

      if(lines.empty())
        lines.emplace_back();

      return lines;
    }
  }
}
