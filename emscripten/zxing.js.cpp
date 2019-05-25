// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  Copyright 2010-2011 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/ReaderException.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/HybridBinarizer.h>
#include <exception>
#include <zxing/Exception.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>
// #include <stdio.h>

#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/multi/qrcode/QRCodeMultiReader.h>
#include <zxing/multi/ByQuadrantReader.h>
#include <zxing/multi/MultipleBarcodeReader.h>
#include <zxing/multi/GenericMultipleBarcodeReader.h>

#include <zxing/LuminanceSource.h>

#include <zxing/Binarizer.h>
#include <zxing/common/BitArray.h>
#include <zxing/common/BitMatrix.h>
#include <zxing/common/Array.h>

#include <emscripten.h>

using namespace std;
using namespace zxing;
using namespace zxing::qrcode;
using namespace zxing::multi;


class ImageReaderSource : public zxing::LuminanceSource {
private:
  typedef LuminanceSource Super;

  const zxing::ArrayRef<char> image;

  char convertPixel(const char* pixel) const;

public:
  ImageReaderSource(zxing::ArrayRef<char> image, int width, int height);

  zxing::ArrayRef<char> getRow(int y, zxing::ArrayRef<char> row) const;
  zxing::ArrayRef<char> getMatrix() const;
};

ImageReaderSource::ImageReaderSource(ArrayRef<char> image_, int width, int height)
    : Super(width, height), image(image_) {}

zxing::ArrayRef<char> ImageReaderSource::getRow(int y, zxing::ArrayRef<char> row) const {
  const char* pixelRow = &image[0] + y * getWidth();
  if (!row) {
    row = zxing::ArrayRef<char>(getWidth());
  }
  for (int x = 0; x < getWidth(); x++) {
    row[x] = pixelRow[x];
  }
  return row;
}

zxing::ArrayRef<char> ImageReaderSource::getMatrix() const {
  return image;
  // zxing::ArrayRef<char> matrix(getWidth() * getHeight());
  // memcpy(&matrix[0], &image[0], getWidth() * getHeight());
  // return matrix;
}


class PassthroughBinarizer : public Binarizer {
private:
  ArrayRef<char> luminances;
public:
  PassthroughBinarizer(Ref<LuminanceSource> source);
  virtual ~PassthroughBinarizer();
    
  virtual Ref<BitArray> getBlackRow(int y, Ref<BitArray> row);
  virtual Ref<BitMatrix> getBlackMatrix();
  Ref<Binarizer> createBinarizer(Ref<LuminanceSource> source);
private:
  void initArrays(int luminanceSize);
};




using zxing::GlobalHistogramBinarizer;
using zxing::Binarizer;
using zxing::ArrayRef;
using zxing::Ref;
using zxing::BitArray;
using zxing::BitMatrix;

// VC++
using zxing::LuminanceSource;

namespace {
  const ArrayRef<char> EMPTY (0);
}

PassthroughBinarizer::PassthroughBinarizer(Ref<LuminanceSource> source) 
  : Binarizer(source), luminances(EMPTY) {}

PassthroughBinarizer::~PassthroughBinarizer() {}

void PassthroughBinarizer::initArrays(int luminanceSize) {
  if (luminances->size() < luminanceSize) {
    luminances = ArrayRef<char>(luminanceSize);
  }
}

Ref<BitArray> PassthroughBinarizer::getBlackRow(int y, Ref<BitArray> row) {
  // std::cerr << "gbr " << y << std::endl;
  LuminanceSource& source = *getLuminanceSource();
  int width = source.getWidth();
  if (row == NULL || static_cast<int>(row->getSize()) < width) {
    row = new BitArray(width);
  } else {
    row->clear();
  }

  initArrays(width);
  ArrayRef<char> localLuminances = source.getRow(y, luminances);
  for (int x = 0; x < width; x++) {
    if (luminances[x]) {
      row->set(x);
    }
  }
  return row;
}
 
Ref<BitMatrix> PassthroughBinarizer::getBlackMatrix() {
  LuminanceSource& source = *getLuminanceSource();
  int width = source.getWidth();
  int height = source.getHeight();
  Ref<BitMatrix> matrix(new BitMatrix(width, height));

  ArrayRef<char> localLuminances = source.getMatrix();
  for (int y = 0; y < height; y++) {
    int offset = y * width;
    for (int x = 0; x < width; x++) {
      if (localLuminances[offset + x]) {
        matrix->set(x, y);
      }
    }
  }
  
  return matrix;
}

Ref<Binarizer> PassthroughBinarizer::createBinarizer(Ref<LuminanceSource> source) {
  return Ref<Binarizer> (new PassthroughBinarizer(source));
}

vector<Ref<Result> > decode_qr_(Ref<BinaryBitmap> image, DecodeHints hints) {
  Ref<Reader> qrCodeReader(new QRCodeReader);
  return vector<Ref<Result> >(1, qrCodeReader->decode(image, hints));
}

vector<Ref<Result> > decode_qr_multi_(Ref<BinaryBitmap> image, DecodeHints hints) {
  Ref<MultipleBarcodeReader> qrCodeMultiReader(new QRCodeMultiReader);
  return qrCodeMultiReader->decodeMultiple(image, hints);
}

vector<Ref<Result> > decode_any_(Ref<BinaryBitmap> image, DecodeHints hints) {
  Ref<Reader> multiFormatReader(new MultiFormatReader);
  return vector<Ref<Result> >(1, multiFormatReader->decode(image, hints));
}

vector<Ref<Result> > decode_multi_(Ref<BinaryBitmap> image, DecodeHints hints) {
  MultiFormatReader delegate;
  GenericMultipleBarcodeReader genericReader(delegate);
  return genericReader.decodeMultiple(image, hints);
}

enum DECODE_MODE {
  QR,
  QR_MULTI,
  ANY,
  MULTI
};


extern "C" {

  static const char *imagePtr = NULL;
  static zxing::ArrayRef<char> image = NULL;
  static Ref<LuminanceSource> source;

  const char* resize(int width, int height) {
    image = zxing::ArrayRef<char>(width*height);
    imagePtr = &image[0];
    source = Ref<LuminanceSource>(new ImageReaderSource(image, width, height));
    return imagePtr;
  }


  int __decode(DECODE_MODE mode) {
    vector<Ref<Result> > results;
    int res = -1;

    Ref<Binarizer> binarizer;
    Ref<BinaryBitmap> binary;

    try {

      DecodeHints hints(DecodeHints::DEFAULT_HINT);

      binarizer = new HybridBinarizer(source);
      binary = new BinaryBitmap(binarizer);

      if (mode == DECODE_MODE::QR) {
        results = decode_qr_(binary, hints);
      } else if (mode == DECODE_MODE::QR_MULTI) {
        results = decode_qr_multi_(binary, hints);
      } else if (mode == DECODE_MODE::ANY) {
        results = decode_any_(binary, hints);
      } else {
        results = decode_multi_(binary, hints);
      }

      res = 0;

    } catch (const ReaderException& e) {
      // printf("%s\n", ("zxing::ReaderException: " + string(e.what())).c_str());
      res = -2;
    } catch (const zxing::IllegalArgumentException& e) {
      // cell_result = "zxing::IllegalArgumentException: " + string(e.what());
      res = -3;
    } catch (const zxing::Exception& e) {
      // cell_result = "zxing::Exception: " + string(e.what());
      res = -4;
    } catch (const std::exception& e) {
      // cell_result = "std::exception: " + string(e.what());
      res = -5;
    }

    EM_ASM({
      ZXing._decode_results = [];
    });

    if (res == 0) {
      for (int i=0; i<results.size(); i++) {
        std::string result = results[i]->getText()->getText();
        auto points = results[i]->getResultPoints();
        int xs[4] = {0}, ys[4] = {0};

        for (int j = 0; j < points->size(); j++) 
        {
            xs[j] = points[j]->getX();
            ys[j] = points[j]->getY();
        }

        EM_ASM_({
					var resultPtr = new Uint8Array(ZXing.HEAPU8.buffer, $0, $1);
          var resultArr = new Uint8Array(resultPtr);
          ZXing._decode_results.push([
            resultArr, 
            $2, $3,
            $4, $5,
            $6, $7
          ]);
        }, 
          result.c_str(), result.size(), 
          xs[0], ys[0],
          xs[1], ys[1],
          xs[2], ys[2],
          xs[3], ys[3]
        );
      }
    }

    return res;
  }


  int decode_qr() {
    return __decode(DECODE_MODE::QR);
  }

  int decode_qr_multi() {
    return __decode(DECODE_MODE::QR_MULTI);
  }

  int decode_any() {
    return __decode(DECODE_MODE::ANY);
  }

  int decode_multi() {
    return __decode(DECODE_MODE::MULTI);
  }

}
