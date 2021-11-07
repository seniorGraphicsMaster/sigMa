#pragma once

#include "cgmath.h"			// slee's simple math library
#include "cgut2.h"

mesh2* load_model( const char* path, bool bFlipUV=false );
void delete_texture_cache(void);