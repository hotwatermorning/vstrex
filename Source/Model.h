#pragma once

#include <GLUT/glut.h>

#include "../JuceLibraryCode/JuceHeader.h"
#include "skeletal_animation_model.hpp"
#include "Util.h"


static Image resizeImageToPowerOfTwo (Image image)
{
    if (! (isPowerOfTwo (image.getWidth()) && isPowerOfTwo (image.getHeight())))
        return image.rescaled (jmin (1024, nextPowerOfTwo (image.getWidth())),
                               jmin (1024, nextPowerOfTwo (image.getHeight())));
    
    return image;
}

struct JUCETexture
{
    JUCETexture (const File& file)
    {
        name = file.getFileName();
        image = resizeImageToPowerOfTwo (ImageFileFormat::loadFrom (file));
        assert(image.isValid());
        texture = std::make_shared<OpenGLTexture>();
        texture->loadImage (image);
    }
    
    void bind() const
    {
        texture->bind();
    }
    
    std::shared_ptr<OpenGLTexture> mutable texture;
    String name;
    Image image;
};

//Class that defines how we are to handle materials, and especially textures, in the 3D model.
//Only diffuse textures stored in external files are handled in this example
class JUCEMaterial : Material {
public:
    std::vector<JUCETexture> diffuseTextures;
    
    JUCEMaterial(const aiMaterial* material) {
        for(unsigned int cdt=0;cdt<material->GetTextureCount(aiTextureType_DIFFUSE);cdt++) {
            aiString path;
            material->GetTexture(aiTextureType_DIFFUSE, cdt, &path); //Assumes external texture files
            auto const texture_file = GetPluginDir().getChildFile("../Resources/model").getChildFile(path.C_Str());
            JUCETexture tx(texture_file);
            diffuseTextures.push_back(tx);
        }
    }
    
    virtual ~JUCEMaterial() {}
    
    void bindTexture(aiTextureType textureType, unsigned int textureId) const {
        diffuseTextures[textureId].bind();
    }
    
    bool texture() const {
        return diffuseTextures.size()>0;
    }
};

//Example 2 - draws a skeletal animated model using SkeletalAnimationModel::drawFrame, which consists of:
//SkeletalAnimationModel::createFrame makes the animation frame given animationId and time,
//SkeletalAnimationModel::getMeshFrame receives the frame vertices and normals for the given mesh, and
//SkeletalAnimationModel::drawMeshFrame draws the given mesh frame. Example 3 and 4 show why we have these 3 functions.
class JUCEModel {
public:
    SkeletalAnimationModel<JUCEMaterial> model; //template argument specifies how we are to handle textures
    
    JUCEModel(File model_path) {
        model.read(model_path.getFullPathName().toStdString());
        assert(model.meshes.size() > 0);
        assert(model.animations.size() > 0);
    }
    
    //Draw the animation frame given time in seconds
    void drawFrame(double time) {
        model.drawFrame(0, time); //this function equals the following lines:
    }
};
