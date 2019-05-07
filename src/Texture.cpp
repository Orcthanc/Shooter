#include "Renderer/Texture.h"

#include <gli/gli.hpp>

using namespace std;
using namespace Shooter::Renderer;

Texture2D::Texture2D( TextureCreateInfo&& cr_inf ){
    device = cr_inf.device;

    gli::texture2d tex2d( gli::load( cr_inf.filename.c_str() ));

    if( tex2d.empty() )
        throw runtime_error( "Could not load texture: " + cr_inf.filename );

    extent.width = static_cast<uint32_t>( tex2d[0].extent().x );
    extent.height = static_cast<uint32_t>( tex2d[0].extent().y );

    miplevels = tex2d.levels();
}
