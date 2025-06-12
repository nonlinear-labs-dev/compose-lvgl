#include "nltools/logging/Log.h"
#include <compose/widgets/SVGImage.h>

void Compose::SVGImage::setModifier(SVGPath p) const
{
  try
  {
    const auto buf = Gdk::Pixbuf::create_from_file(p.it);
    if(p.size.has_value())
    {
      const auto [w, h] = p.size.value();
      const auto scaled_buf = buf->scale_simple(w, h, Gdk::INTERP_BILINEAR);
      getHandle()->set(scaled_buf);
    }
    else
    {
      getHandle()->set(buf);
    }
  }
  catch(Glib::FileError &e)
  {
    nltools::Log::error("could not load", p.it.string(), e.what());
  }
}

void Compose::SVGImage::setModifier(PixelSize s) const
{
  getHandle()->set_pixel_size(s.it);
}

void Compose::SVGImage::setModifier(PrimaryColor c) const
{
  if(const auto buffer = getHandle()->get_pixbuf())
  {
    const auto width = buffer->get_width();
    const auto height = buffer->get_height();

    if(const auto workingCopy = Gdk::Pixbuf::create(buffer->get_colorspace(), buffer->get_has_alpha(),
                                                    buffer->get_bits_per_sample(), width, height))
    {
      auto [r, g, b, a] = c;
      const auto srcPixels = buffer->get_pixels();
      const auto dstPixels = workingCopy->get_pixels();
      const auto rowStride = buffer->get_rowstride();
      const auto nChannels = buffer->get_n_channels();

      for(int y = 0; y < height; ++y)
      {
        for(int x = 0; x < width; ++x)
        {
          const auto srcPixel = srcPixels + y * rowStride + x * nChannels;
          const auto dstPixel = dstPixels + y * rowStride + x * nChannels;

          if(nChannels == 4)
          {
            if(srcPixel[3] != 0)
            {
              dstPixel[0] = r;
              dstPixel[1] = g;
              dstPixel[2] = b;
              dstPixel[3] = srcPixel[3];
            }
            else
            {
              dstPixel[0] = srcPixel[0];
              dstPixel[1] = srcPixel[1];
              dstPixel[2] = srcPixel[2];
              dstPixel[3] = srcPixel[3];
            }
          }
          else if(nChannels == 3)
          {
            dstPixel[0] = r;
            dstPixel[1] = g;
            dstPixel[2] = b;
          }
        }
      }

      getHandle()->set(workingCopy);
    }
  }
}