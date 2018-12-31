#include <mutex>
#include <chrono>
#if defined(_MSC_VER)
#include <GL/freeglut.h>
#else
#include <GLUT/glut.h>
#endif

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Model.h"

using clock_type = std::chrono::steady_clock;
double get_clock_sec()
{
    return std::chrono::duration<double>(clock_type::now().time_since_epoch()).count();
}

#if !defined(_MSC_VER)
void setOpenGLContextSurfaceOpacityToZero(OpenGLContext *context);
#endif

static constexpr int kImageWidth = 642;
static constexpr int kImageHeight = 180;
static constexpr int kAnimationFrameBegin = 1010;
static constexpr int kAnimationFrameLength = 20;
static constexpr double kDefaultScale = 0.7;

struct RenderingParameterSet
{
    std::function<double()> get_rms_;
    std::function<double()> get_rotation_speed_;
    std::function<double()> get_running_speed_;
};

struct LockableImage
{
public:
    LockableImage(int width, int height)
    :   image_(Image::ARGB, width, height, true)
    {}
    
    operator Image & () { return image_; }
    operator Image const & () const { return image_; }
    
    Image & AsImage() { return image_; }
    Image const & AsImage() const { return image_; }
    
    int getWidth() const { return image_.getWidth(); }
    int getHeight() const { return image_.getHeight(); }
  
    bool try_lock() const { return mtx_.try_lock(); }
    void lock() const { mtx_.lock(); }
    void unlock() const { mtx_.unlock(); }
    
private:
    std::mutex mutable mtx_;
    Image image_;
};

//==============================================================================
//! OpenGLの描画用。
//! MacではOpenGL描画を有効にしていると透過部分であってもマウスクリックが透過しないので、
//! まずOpenGL描画専用にマウスクリックを完全に無視するウィンドウを作成し、
//! そこで描画したデータを別ウィンドウで画像として描画する。
class RenderingComponent
:   public Component
,   public OpenGLRenderer
{
public:
    //==============================================================================
    RenderingComponent(LockableImage *image, RenderingParameterSet param);
    ~RenderingComponent();
    
    //bool hitTest(int x, int y) override { return false; }
    
    void paint(Graphics &g) override {
        g.fillAll(juce::Colours::transparentBlack);
    }
    
private:
    //==============================================================================
    // Your private member variables go here...
    
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    OpenGLContext openGLContext;
    
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;
    
    void EnableContinuousRendering(bool flag = true);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RenderingComponent)
};

struct RenderingComponent::Impl
{
    double last_time_;
    bool initialized_ = false;
    OpenGLFrameBuffer frame_buffer_;
    LockableImage *image_;
    std::unique_ptr<JUCEModel> model_;
    RenderingParameterSet params_;
    double rotation_angle_ = 0;
    double running_time_ = 0;
};

RenderingComponent::RenderingComponent(LockableImage *image, RenderingParameterSet params)
:   pimpl_(std::make_unique<Impl>())
{
    pimpl_->image_ = image;
    pimpl_->params_ = params;
    
    setInterceptsMouseClicks(false, false);
    
    setBufferedToImage(true);
    setWantsKeyboardFocus(true);
    openGLContext.setComponentPaintingEnabled(false);
    openGLContext.setContinuousRepainting (true);
    //openGLContext.setOpenGLVersionRequired(OpenGLContext::OpenGLContext::openGL3_2);
    openGLContext.setPixelFormat(OpenGLPixelFormat(8, 8, 24, 8));
    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
}

RenderingComponent::~RenderingComponent()
{
    openGLContext.detach();
}

void RenderingComponent::newOpenGLContextCreated()
{
#if !defined(_MSC_VER)
    setOpenGLContextSurfaceOpacityToZero(&openGLContext);
#endif
    
    // load
    auto model_path = GetModelPath();
    pimpl_->model_ = std::make_unique<JUCEModel>(model_path);
    
    int const w = getWidth();
    int const h = getHeight();
    
    //glEnable(GL_MULTISAMPLE);
    glViewport(0, 0, w, h);
    {
        auto lock = std::unique_lock<LockableImage>(*pimpl_->image_);
        pimpl_->image_->AsImage() = Image(Image::ARGB, w, h, true);
    }
    
    pimpl_->last_time_ = get_clock_sec();
    
    pimpl_->frame_buffer_.initialise(openGLContext, w, h);
    pimpl_->initialized_ = true;
}

void RenderingComponent::renderOpenGL()
{
    auto &frame_buffer = pimpl_->frame_buffer_;
    frame_buffer.makeCurrentAndClear();
    
    // This clears the context with a black background.
    OpenGLHelpers::clear (Colours::transparentBlack);
    
    auto const new_time = get_clock_sec();
    
    pimpl_->running_time_ += (new_time - pimpl_->last_time_) * pimpl_->params_.get_running_speed_();
    pimpl_->rotation_angle_ += (new_time - pimpl_->last_time_) * pimpl_->params_.get_rotation_speed_();
    pimpl_->last_time_ = new_time;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    
    //Setup projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    gluPerspective(30.0, kImageWidth / (double)kImageHeight, 0.1, 100.0);

    //double x = JUCE_LIVE_CONSTANT(40.0);
    double const x = 37.0;
    gluLookAt(0.0, 5.0, x, //Camera position in World Space
              0.0, 8.0, 0.0,    //Camera looks towards this position
              0.0, 1.0, 0.0);   //Up
    
    glMatrixMode(GL_MODELVIEW);
    
    glPushMatrix();
    glRotatef(pimpl_->rotation_angle_ * 360, 0.0, 1.0, 0.0);
    auto const scale = 150 * (pimpl_->params_.get_rms_() * (1 - kDefaultScale) + kDefaultScale);
	//glTranslatef(0, 0, 30);
    glScalef(scale, scale, scale);
    pimpl_->model_->drawFrame(fmod(pimpl_->running_time_, (kAnimationFrameLength / 30.0)) + (kAnimationFrameBegin / 30.0));
    glPopMatrix();
    
    auto rect = getLocalBounds();
    
    {
        auto lock = std::unique_lock<LockableImage>(*pimpl_->image_);
        Image::BitmapData bitmap(*pimpl_->image_, 0, 0, rect.getWidth(), rect.getHeight());
        frame_buffer.readPixels((PixelARGB*)bitmap.data, rect);
    }
    frame_buffer.releaseAsRenderingTarget();
    
    OpenGLHelpers::clear(Colours::transparentBlack);
    glFlush();
}

void RenderingComponent::openGLContextClosing()
{
    pimpl_->initialized_ = false;
    pimpl_->model_.reset();
}

void RenderingComponent::EnableContinuousRendering(bool flag)
{
    openGLContext.setComponentPaintingEnabled(flag);
}

struct RenderingWindow : DocumentWindow
{
public:
    RenderingWindow(LockableImage *image, RenderingParameterSet params)
    :   DocumentWindow ("RenderingWindow",
                        Colours::transparentBlack,
                        DocumentWindow::allButtons,
                        true)
    {
        setBufferedToImage(true);
        setSize(kImageWidth, kImageHeight);
        setUsingNativeTitleBar (false);
        setTitleBarHeight(0);
        auto component = new RenderingComponent(image, params);
        component->setSize(getWidth(), getHeight());
        setContentOwned (component, true);

        //centreWithSize (getWidth(), getHeight());
        //setInterceptsMouseClicks(false, false);
        setVisible (true);
        getContentComponent()->setVisible(true);
        //setAlwaysOnTop(true);
        setOpaque(false);
    }

    int getDesktopWindowStyleFlags() const override
    {
        auto styleFlags = DocumentWindow::getDesktopWindowStyleFlags();
        styleFlags |= ComponentPeer::windowIgnoresMouseClicks;
        styleFlags &= ~(ComponentPeer::windowHasDropShadow);
        return styleFlags;
    }
    
    void Detach()
    {
        clearContentComponent();
    }
    
    void paint(Graphics &g) override {
        g.fillAll(juce::Colours::transparentBlack);
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RenderingWindow)
};

struct DrawingWindow
:   public DocumentWindow
,   public Timer
{
public:
    DrawingWindow (LockableImage const *image)  : DocumentWindow ("DrawingWindow",
                                                                  Colours::transparentBlack,
                                                                  DocumentWindow::allButtons,
                                                                  true)
    {
        assert(image);
        
        setSize(kImageWidth, kImageHeight);
        setUsingNativeTitleBar (false);
        setTitleBarHeight(0);
        
        centreWithSize (getWidth(), getHeight());
    
        setVisible (true);
        setAlwaysOnTop(true);
        setOpaque(false);
        startTimerHz(60);
        mirror_ = Image(Image::ARGB, kImageWidth, kImageHeight, true);
        image_ = image;
        //setBufferedToImage(true);
    }
    
    void timerCallback() override
    {
        repaint();
    }
    
    LockableImage const *image_;
    Image mirror_;
    
    void paint(Graphics &g) override
    {
        {
            mirror_.clear(mirror_.getBounds());
            Graphics g2 (mirror_);
            
            auto lock = std::unique_lock<LockableImage const>(*image_);
            AffineTransform transform4mirroring = AffineTransform::scale (1.0f, -1.0f);
            transform4mirroring = transform4mirroring.translated (0.0f, (*image_).getHeight());
            g2.drawImageTransformed (*image_, transform4mirroring);
        }
        
//        g.fillAll(juce::Colours::blue.withAlpha(0.1f));
//        g.fillAll(juce::Colours::transparentBlack);
        g.drawImage(mirror_, getLocalBounds().toFloat());
        
        //g.fillAll(juce::Colours::transparentBlack);
        //g.setColour(juce::Colours::black);
        //g.drawEllipse(getLocalBounds().reduced(10).toFloat(), 10);
    }
    
    int getDesktopWindowStyleFlags() const override
    {
        auto styleFlags = DocumentWindow::getDesktopWindowStyleFlags();
        styleFlags &= ~ComponentPeer::windowHasDropShadow;
        styleFlags |= ComponentPeer::windowIsSemiTransparent;
        //styleFlags |= ComponentPeer::windowIsTemporary;
        return styleFlags;
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrawingWindow)
};

struct VstrexAudioProcessorEditor::Impl
{
    Impl()
    :   image_(kImageWidth, kImageHeight)
    {}
    
    Image logo_;
    std::unique_ptr<RenderingWindow> renderer_;
    std::unique_ptr<DrawingWindow> drawer_;
    LockableImage image_;
    bool show_credit_ = false;
};

//==============================================================================
VstrexAudioProcessorEditor::VstrexAudioProcessorEditor (VstrexAudioProcessor& p)
:   AudioProcessorEditor (&p)
,   processor (p)
,   pimpl_(std::make_unique<Impl>())
{
    RenderingParameterSet params;
    params.get_rms_ = [this] { return processor.getRMS(); };
    params.get_rotation_speed_ = [this] { return processor.rotation_speed_->get(); };
    params.get_running_speed_ = [this] { return processor.running_speed_->get() * processor.tempo_.load() / 120.0; };
    
    pimpl_->renderer_ = std::make_unique<RenderingWindow>(&pimpl_->image_, params);
    pimpl_->drawer_ = std::make_unique<DrawingWindow>(&pimpl_->image_);
    setSize (400, 160);
    auto logo_file = GetLogoPath();
    pimpl_->logo_ = ImageFileFormat::loadFrom(logo_file);
}

VstrexAudioProcessorEditor::~VstrexAudioProcessorEditor()
{
//    if(pimpl_->renderer_) {
//        pimpl_->renderer_->Detach();
//    }
}

//==============================================================================
void VstrexAudioProcessorEditor::paint (Graphics& g)
{
    if(pimpl_->logo_.isValid()) {
        g.drawImage(pimpl_->logo_, getLocalBounds().toFloat());
    } else {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        
        g.setColour (Colours::white);
        g.setFont (15.0f);
        g.drawFittedText ("Failed to load Logo Image...", getLocalBounds(), Justification::centred, 1);
    }
    
    if(pimpl_->show_credit_) {
        g.fillAll (Colours::black.withAlpha(0.6f));
        g.setColour (Colours::white);
        g.setFont (12.0f);
        std::string credit =
        "VST.rex by https://github.com/hotwatermorning\n"
        "\n"
        "Uses following libraries:\n"
        "\n"
        "The Open-Asset-Importer-Lib\n"
        "    http://www.assimp.org/\n"
        "Skeletal-Animation-Library\n"
        "    https://gitlab.com/eidheim/Skeletal-Animation-Library/\n"
        "freeglut\n"
        "    http://freeglut.sourceforge.net/";
        g.drawFittedText (credit, getLocalBounds().reduced(10), Justification::topLeft, 1);
    }
}

void VstrexAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void VstrexAudioProcessorEditor::mouseUp(MouseEvent const &ev)
{
    pimpl_->show_credit_ = !pimpl_->show_credit_;
    repaint();
}
