/* SPDX-FileCopyrightText: 2011-2022 Blender Foundation
 *
 * SPDX-License-Identifier: Apache-2.0 */

#include "anari/anari_cpp.hpp"

#include "ImageLoader.h"

namespace cycles {

ANARIImageLoader::ANARIImageLoader(const StructuredRegularField *field_ptr) : p_field(field_ptr) {}

ANARIImageLoader::~ANARIImageLoader() {}

bool ANARIImageLoader::load_metadata(const ImageDeviceFeatures &features, ImageMetaData &metadata)
{
  metadata.byte_size = p_field->m_data->totalSize() * anari::sizeOf(p_field->m_data->elementType());
  metadata.channels = 3;
  //TODO
  Transform t;
  for (int col = 0; col < 4; col++) {
    for (int row = 0; row < 3; row++) {
      t[row][col] = (row == col) ? 1 : 0;
    }
  }
  metadata.transform_3d = t;
  metadata.use_transform_3d = true;

  metadata.width  = p_field->m_dims[0];
  metadata.height = p_field->m_dims[1];
  metadata.depth  = p_field->m_dims[2];

  switch (p_field->m_data->elementType()) {
    case (ANARI_UFIXED8):
      metadata.type = IMAGE_DATA_TYPE_BYTE;
      break;
    case (ANARI_UFIXED16):
      metadata.type = IMAGE_DATA_TYPE_USHORT;
      break;
    case (ANARI_FLOAT32):
      metadata.type = IMAGE_DATA_TYPE_FLOAT;
      break;
    case (ANARI_FIXED16):
    case (ANARI_FLOAT64):
    case (ANARI_UNKNOWN):
      //TODO throw error
      std::cerr << "Unsupported voxel data type\n";
      return false;
  }

  return true;
}

bool ANARIImageLoader::load_pixels(const ImageMetaData &, void *pixels, const size_t, const bool)
{
  auto size = p_field->m_data->totalSize() * anari::sizeOf(p_field->m_data->elementType());
  memcpy(pixels, p_field->m_data->data(), size);
  return true;
}

string ANARIImageLoader::name() const
{
  return "ANARI Volume";
}

bool ANARIImageLoader::equals(const ImageLoader &other) const
{
  //TODO
  return false;
}

void ANARIImageLoader::cleanup()
{
}

bool ANARIImageLoader::is_vdb_loader() const
{
  return false;
}

} // namespace cycles
