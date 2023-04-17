/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making libpag available.
//
//  Copyright (C) 2023 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include "framework/pag_test.h"
#include "framework/utils/PAGTestUtils.h"
#include "platform/Platform.h"
#include "rendering/caches/DiskCache.h"
#include "rendering/utils/Directory.h"

namespace pag {

PAG_TEST_SUIT(PAGDiskCacheTest)

//PAG_TEST_F(PAGDiskCacheTest, GenerateTestCaches) {
//  auto cacheDir = Platform::Current()->getCacheDir();
//  std::filesystem::remove_all(cacheDir);
//  auto pagFile = LoadPAGFile("resources/apitest/ZC2.pag");
//  tgfx::Bitmap bitmap(pagFile->width(), pagFile->height());
//  tgfx::Pixmap pixmap(bitmap);
//  auto pagPlayer = std::make_shared<PAGPlayer>();
//  pagPlayer->setComposition(pagFile);
//  auto pagSurface = PAGSurface::MakeOffscreen(pagFile->width(), pagFile->height());
//  pagPlayer->setSurface(pagSurface);
//  auto info =
//      tgfx::ImageInfo::Make(pagFile->width(), pagFile->height(), tgfx::ColorType::RGBA_8888);
//  auto sequenceFile =
//      DiskCache::OpenSequence("resources/apitest/ZC2.pag", info, 30, pagFile->frameRate());
//  sequenceFile =
//      DiskCache::OpenSequence("resources/apitest/ZC2.pag.720x1280", info, 30, pagFile->frameRate());
//  for (auto i = 0; i < 11; i++) {
//    pagPlayer->flush();
//    pagSurface->readPixels(ColorType::RGBA_8888, AlphaType::Premultiplied, pixmap.writablePixels(),
//                           pixmap.rowBytes());
//    sequenceFile->writeFrame(i, pixmap.pixels(), pixmap.byteSize());
//    pagPlayer->nextFrame();
//  }
//
//  info = tgfx::ImageInfo::Make(360, 640, tgfx::ColorType::RGBA_8888);
//  pixmap.reset();
//  bitmap.allocPixels(info.width(), info.height());
//  pixmap.reset(bitmap);
//  pagSurface = PAGSurface::MakeOffscreen(info.width(), info.height());
//  pagPlayer->setSurface(pagSurface);
//  sequenceFile =
//      DiskCache::OpenSequence("resources/apitest/ZC2.pag.360x640", info, 30, pagFile->frameRate());
//  pagPlayer->setProgress(0);
//  for (auto i = 0; i < 30; i++) {
//    pagPlayer->flush();
//    pagSurface->readPixels(ColorType::RGBA_8888, AlphaType::Premultiplied, pixmap.writablePixels(),
//                           pixmap.rowBytes());
//    sequenceFile->writeFrame(i, pixmap.pixels(), pixmap.byteSize());
//    pagPlayer->nextFrame();
//  }
//  sequenceFile = nullptr;
//
//  auto diskDir = TestConstants::PAG_ROOT + "resources/disk/libpag";
//  std::filesystem::remove_all(diskDir);
//  std::filesystem::create_directories(diskDir);
//  std::filesystem::copy(cacheDir, diskDir, std::filesystem::copy_options::recursive);
//  std::filesystem::remove(diskDir + "/files/1.bin");
//  std::filesystem::copy(diskDir + "/files/3.bin", diskDir + "/files/4.bin");
//  std::filesystem::remove_all(cacheDir);
//}

/**
 * 用例描述: 测试 SequenceFile 的磁盘缓存功能。
 */
PAG_TEST_F(PAGDiskCacheTest, SequenceFile) {
  auto cacheDir = Platform::Current()->getCacheDir();
  std::filesystem::remove_all(cacheDir);
  std::filesystem::create_directories(cacheDir);
  std::filesystem::copy(TestConstants::PAG_ROOT + "resources/disk/libpag", cacheDir,
                        std::filesystem::copy_options::recursive);
  auto pagFile = LoadPAGFile("resources/apitest/ZC2.pag");
  ASSERT_TRUE(pagFile != nullptr);
  auto info =
      tgfx::ImageInfo::Make(pagFile->width(), pagFile->height(), tgfx::ColorType::RGBA_8888);
  auto sequenceFile =
      DiskCache::OpenSequence("resources/apitest/ZC2.pag.720x1280", info, 30, pagFile->frameRate());
  ASSERT_TRUE(sequenceFile != nullptr);
  EXPECT_EQ(sequenceFile->width(), pagFile->width());
  EXPECT_EQ(sequenceFile->height(), pagFile->height());
  EXPECT_EQ(sequenceFile->numFrames(), 30u);
  EXPECT_EQ(sequenceFile->frameRate(), pagFile->frameRate());
  EXPECT_FALSE(sequenceFile->isComplete());
  EXPECT_EQ(sequenceFile->cachedFrames, 11);
  auto diskCache = sequenceFile->diskCache;
  EXPECT_FALSE(std::filesystem::exists(cacheDir + "/files/4.bin"));
  EXPECT_TRUE(diskCache->openedFiles.size() == 1);
  EXPECT_TRUE(diskCache->cachedFileMap.size() == 2);
  EXPECT_EQ(diskCache->cachedFiles.size(), 2u);
  EXPECT_EQ(diskCache->fileIDCount, 4u);
  const auto InitialDiskSize = 647620u;
  EXPECT_EQ(diskCache->totalDiskSize, InitialDiskSize);

  tgfx::Bitmap bitmap(pagFile->width(), pagFile->height());
  tgfx::Pixmap pixmap(bitmap);
  auto success = sequenceFile->readFrame(10, pixmap.writablePixels());
  EXPECT_TRUE(success);
  EXPECT_TRUE(Baseline::Compare(pixmap, "PAGDiskCacheTest/SequenceFile_10"));
  success = sequenceFile->readFrame(15, pixmap.writablePixels());
  EXPECT_FALSE(success);
  success = sequenceFile->readFrame(20, pixmap.writablePixels());
  EXPECT_FALSE(success);

  auto pagPlayer = std::make_shared<PAGPlayer>();
  pagPlayer->setComposition(pagFile);
  auto pagSurface = PAGSurface::MakeOffscreen(pagFile->width(), pagFile->height());
  pagPlayer->setSurface(pagSurface);
  auto initialFileSize = sequenceFile->fileSize();
  for (auto i = 0; i < 30; i++) {
    pagPlayer->flush();
    success = pagSurface->readPixels(ColorType::RGBA_8888, AlphaType::Premultiplied,
                                     pixmap.writablePixels(), pixmap.rowBytes());
    ASSERT_TRUE(success);
    success = sequenceFile->writeFrame(i, pixmap.pixels());
    if (i < 11) {
      EXPECT_FALSE(success);
      EXPECT_EQ(diskCache->totalDiskSize, InitialDiskSize);
    }
    pagPlayer->nextFrame();
  }
  EXPECT_EQ(diskCache->totalDiskSize, InitialDiskSize + sequenceFile->fileSize() - initialFileSize);
  EXPECT_TRUE(sequenceFile->isComplete());
  EXPECT_TRUE(sequenceFile->encoder == nullptr);
  success = sequenceFile->readFrame(15, pixmap.writablePixels());
  EXPECT_TRUE(success);
  EXPECT_TRUE(Baseline::Compare(pixmap, "PAGDiskCacheTest/SequenceFile_15"));
  success = sequenceFile->readFrame(30, pixmap.writablePixels());
  EXPECT_FALSE(success);

  info = tgfx::ImageInfo::Make(360, 640, tgfx::ColorType::RGBA_8888);
  pixmap.reset();
  bitmap.allocPixels(info.width(), info.height());
  pixmap.reset(bitmap);
  auto halfSequenceFile =
      DiskCache::OpenSequence("resources/apitest/ZC2.pag.360x640", info, 30, pagFile->frameRate());
  ASSERT_TRUE(halfSequenceFile != nullptr);
  EXPECT_EQ(halfSequenceFile->width(), 360);
  EXPECT_EQ(halfSequenceFile->height(), 640);
  EXPECT_EQ(halfSequenceFile->numFrames(), 30u);
  EXPECT_EQ(halfSequenceFile->frameRate(), pagFile->frameRate());
  EXPECT_TRUE(halfSequenceFile->isComplete());
  success = halfSequenceFile->readFrame(20, pixmap.writablePixels());
  EXPECT_TRUE(success);
  EXPECT_TRUE(Baseline::Compare(pixmap, "PAGDiskCacheTest/SequenceFile_20"));
  const auto halfSequenceFileSize = halfSequenceFile->fileSize();
  halfSequenceFile = nullptr;

  info = tgfx::ImageInfo::Make(540, 960, tgfx::ColorType::RGBA_8888);
  pagSurface = PAGSurface::MakeOffscreen(info.width(), info.height());
  pagPlayer->setSurface(pagSurface);
  pixmap.reset();
  bitmap.allocPixels(info.width(), info.height());
  pixmap.reset(bitmap);

  const auto lastTotalDiskSize = diskCache->totalDiskSize;

  DiskCache::SetMaxDiskSize(1800000u);
  EXPECT_EQ(DiskCache::MaxDiskSize(), 1800000u);
  sequenceFile =
      DiskCache::OpenSequence("resources/apitest/ZC2.pag.540x960", info, 30, pagFile->frameRate());
  pagPlayer->setProgress(0);
  for (auto i = 0; i < 30; i++) {
    pagPlayer->flush();
    success = pagSurface->readPixels(ColorType::RGBA_8888, AlphaType::Premultiplied,
                                     pixmap.writablePixels(), pixmap.rowBytes());
    ASSERT_TRUE(success);
    sequenceFile->writeFrame(i, pixmap.pixels());
    pagPlayer->nextFrame();
  }
  success = sequenceFile->readFrame(22, pixmap.writablePixels());
  EXPECT_TRUE(success);
  EXPECT_TRUE(Baseline::Compare(pixmap, "PAGDiskCacheTest/SequenceFile_22"));
  EXPECT_EQ(diskCache->totalDiskSize,
            lastTotalDiskSize + sequenceFile->fileSize() - halfSequenceFileSize);
  EXPECT_FALSE(std::filesystem::exists(cacheDir + "/files/3.bin"));
  EXPECT_TRUE(std::filesystem::exists(cacheDir + "/files/2.bin"));
  DiskCache::SetMaxDiskSize(0);
  EXPECT_EQ(diskCache->totalDiskSize, sequenceFile->fileSize());
  EXPECT_FALSE(std::filesystem::exists(cacheDir + "/files/2.bin"));
  EXPECT_TRUE(std::filesystem::exists(cacheDir + "/files/4.bin"));

  auto sequenceFile2 =
      DiskCache::OpenSequence("resources/apitest/ZC2.pag.540x960", info, 30, pagFile->frameRate());
  EXPECT_EQ(sequenceFile2, sequenceFile);
  auto sequenceFile3 = DiskCache::OpenSequence("resources/apitest/ZC2.pag.540x960", info, 30,
                                               pagFile->frameRate() * 0.5f);
  EXPECT_NE(sequenceFile3, sequenceFile);
  EXPECT_EQ(diskCache->totalDiskSize, 0u);
  EXPECT_TRUE(std::filesystem::exists(cacheDir + "/files/4.bin"));
  EXPECT_TRUE(std::filesystem::exists(cacheDir + "/files/5.bin"));
  sequenceFile = nullptr;
  sequenceFile2 = nullptr;
  EXPECT_FALSE(std::filesystem::exists(cacheDir + "/files/4.bin"));
  DiskCache::SetMaxDiskSize(1073741824);  // 1GB
  std::filesystem::remove_all(cacheDir);
}

PAG_TEST_F(PAGDiskCacheTest, PAGDecoder) {
  auto pagFile = LoadPAGFile("resources/apitest/data_bmp.pag");
  ASSERT_TRUE(pagFile != nullptr);
  auto decoder = PAGDecoder::MakeFrom(pagFile, 30, 0.5f);
  ASSERT_TRUE(decoder != nullptr);
  EXPECT_EQ(decoder->width(), pagFile->width() / 2);
  EXPECT_EQ(decoder->height(), pagFile->height() / 2);
  EXPECT_EQ(decoder->numFrames(), 95);
  EXPECT_EQ(decoder->frameRate(), 24.0f);
  pagFile = nullptr;
  tgfx::Bitmap bitmap(decoder->width(), decoder->height());
  tgfx::Pixmap pixmap(bitmap);
  for (int i = 0; i < decoder->numFrames(); i++) {
    auto success = decoder->readFrame(i, pixmap.writablePixels(), pixmap.rowBytes());
    EXPECT_TRUE(success);
  }
  EXPECT_TRUE(decoder->sequenceFile->isComplete());
  EXPECT_TRUE(decoder->sequenceFile != nullptr);
  EXPECT_TRUE(decoder->pagPlayer == nullptr);
  EXPECT_TRUE(decoder->getComposition() == nullptr);
  auto success = decoder->readFrame(50, pixmap.writablePixels(), pixmap.rowBytes());
  EXPECT_TRUE(success);
  EXPECT_TRUE(Baseline::Compare(pixmap, "PAGDiskCacheTest/decoder_frame_50"));
  auto decoder2 = PAGDecoder::MakeFrom(pagFile, 30, 0.5f);
  EXPECT_TRUE(decoder2 == nullptr);
  pagFile = LoadPAGFile("resources/apitest/data_bmp.pag");
  decoder2 = PAGDecoder::MakeFrom(pagFile, 30, 0.5f);
  ASSERT_TRUE(decoder2 != nullptr);
  success = decoder2->readFrame(50, pixmap.writablePixels(), pixmap.rowBytes());
  EXPECT_TRUE(success);
  EXPECT_EQ(decoder->sequenceFile, decoder2->sequenceFile);
  EXPECT_TRUE(decoder2->pagPlayer == nullptr);
  EXPECT_TRUE(decoder2->getComposition() != nullptr);
  success =
      decoder2->readFrame(50, pixmap.writablePixels(), pixmap.rowBytes(), ColorType::BGRA_8888);
  EXPECT_FALSE(success);
  success = decoder2->readFrame(50, pixmap.writablePixels(), pixmap.rowBytes(),
                                ColorType::RGBA_8888, AlphaType::Unpremultiplied);
  EXPECT_FALSE(success);
  success = decoder2->readFrame(50, pixmap.writablePixels(), pixmap.rowBytes() - 1);
  EXPECT_FALSE(success);
  success = decoder2->readFrame(-1, pixmap.writablePixels(), pixmap.rowBytes());
  EXPECT_FALSE(success);
  success = decoder2->readFrame(120, pixmap.writablePixels(), pixmap.rowBytes());
  EXPECT_FALSE(success);

  auto decoder3 = PAGDecoder::MakeFrom(pagFile, 30, 0.5f);
  EXPECT_TRUE(decoder3 != nullptr);
  EXPECT_TRUE(decoder3->getComposition() != nullptr);
  EXPECT_TRUE(decoder2->getComposition() == nullptr);
  success = decoder3->readFrame(0, pixmap.writablePixels(), pixmap.rowBytes(), ColorType::BGRA_8888,
                                AlphaType::Unpremultiplied);
  EXPECT_TRUE(success);
  EXPECT_TRUE(decoder3->getComposition() != nullptr);
  auto info = tgfx::ImageInfo::Make(pixmap.width(), pixmap.height(), tgfx::ColorType::BGRA_8888,
                                    tgfx::AlphaType::Unpremultiplied);
  EXPECT_TRUE(
      Baseline::Compare(tgfx::Pixmap(info, pixmap.pixels()), "PAGDiskCacheTest/decoder_frame_0"));
  EXPECT_NE(decoder->sequenceFile, decoder3->sequenceFile);
  EXPECT_TRUE(decoder3->pagPlayer != nullptr);
  EXPECT_TRUE(decoder3->getComposition() != nullptr);

  auto pagFile2 = LoadPAGFile("resources/apitest/ZC2.pag");
  ASSERT_TRUE(pagFile2 != nullptr);
  auto composition = PAGComposition::Make(pagFile2->width(), pagFile2->height());
  composition->addLayer(pagFile2);
  composition->addLayer(pagFile);
  success = decoder3->readFrame(1, pixmap.writablePixels(), pixmap.rowBytes(), ColorType::BGRA_8888,
                                AlphaType::Unpremultiplied);
  EXPECT_FALSE(success);
  auto decoder4 = PAGDecoder::MakeFrom(composition);
  ASSERT_TRUE(decoder4 != nullptr);
  EXPECT_EQ(decoder4->height(), pagFile2->height());
  EXPECT_EQ(decoder4->width(), pagFile2->width());
  EXPECT_EQ(decoder4->numFrames(), 119);
  EXPECT_EQ(decoder4->frameRate(), 30.0f);
  pixmap.reset();
  bitmap.allocPixels(pagFile2->width(), pagFile2->height());
  pixmap.reset(bitmap);
  success = decoder4->readFrame(15, pixmap.writablePixels(), pixmap.rowBytes());
  EXPECT_TRUE(success);
  EXPECT_TRUE(Baseline::Compare(pixmap, "PAGDiskCacheTest/decoder_frame_15"));
  composition->removeLayerAt(1);
  EXPECT_EQ(decoder4->numFrames(), 30);
  EXPECT_EQ(decoder4->frameRate(), 24.0f);
  success = decoder4->readFrame(12, pixmap.writablePixels(), pixmap.rowBytes());
  EXPECT_TRUE(success);
  EXPECT_TRUE(Baseline::Compare(pixmap, "PAGDiskCacheTest/decoder_frame_12"));

  auto decoder5 = PAGDecoder::MakeFrom(composition);
  EXPECT_TRUE(decoder5 != nullptr);
  success = decoder4->readFrame(12, pixmap.writablePixels(), pixmap.rowBytes());
  EXPECT_TRUE(success);
  success = decoder4->readFrame(13, pixmap.writablePixels(), pixmap.rowBytes());
  EXPECT_FALSE(success);
  success = decoder5->readFrame(12, pixmap.writablePixels(), pixmap.rowBytes());
  EXPECT_TRUE(success);

  auto cacheDir = Platform::Current()->getCacheDir();
  auto files = Directory::FindFiles(cacheDir + "/files", ".bin");
  auto diskFileCount = files.size();
  decoder = nullptr;
  files = Directory::FindFiles(cacheDir + "/files", ".bin");
  EXPECT_EQ(files.size(), diskFileCount);
  decoder2 = nullptr;
  files = Directory::FindFiles(cacheDir + "/files", ".bin");
  EXPECT_EQ(files.size(), diskFileCount - 1);
  decoder3 = nullptr;
  files = Directory::FindFiles(cacheDir + "/files", ".bin");
  EXPECT_EQ(files.size(), diskFileCount - 1);
  decoder4 = nullptr;
  files = Directory::FindFiles(cacheDir + "/files", ".bin");
  EXPECT_EQ(files.size(), diskFileCount - 2);
  decoder5 = nullptr;
  files = Directory::FindFiles(cacheDir + "/files", ".bin");
  EXPECT_EQ(files.size(), diskFileCount - 3);

  std::filesystem::remove_all(cacheDir);
}

}  // namespace pag
