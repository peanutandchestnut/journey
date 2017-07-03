#ifndef BLENDERVIEW_H
#define BLENDERVIEW_H

#include <osgViewer/View>
#include <osgText/Text>
#include "auxiliary.h"
#include "rect.h"
#include "zmath.h"
#include "gridfloor.h"

namespace zxd {

class Blender;

/*
 * copy world axes in target camera rotation
 */
class MiniAxesCallback : public osg::NodeCallback {
protected:
  osg::Camera* mTargetCamera;

public:
  osg::Camera* getTargetCamera() const { return mTargetCamera; }
  void setTargetCamera(osg::Camera* v) { mTargetCamera = v; }

protected:
  virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
};

/*
 * each view has it's and hudcamera,  used to add view info, miniworld axis and
 * ortho grid. All view share the same scene data.
 * You can rotate translate scale selection in any view.
 * Ortho view is fixed, you can only reverse it. (eg : front to back)
 */

class BlenderView : public osgViewer::View {
public:
  enum ViewType { VT_ORTHO, VT_USER };

protected:
  GLfloat mMiniAxesSize;  // scale size

  osg::ref_ptr<osg::Camera> mHudCamera;
  osg::ref_ptr<osgText::Text> mText;
  osg::ref_ptr<zxd::Axes> mMiniAxes;
  osg::ref_ptr<zxd::GridFloor> mGridFloor;
  osg::ref_ptr<osg::Group> mRoot;
  ViewType mViewType;

public:
  BlenderView(osg::GraphicsContext* gc, ViewType type);

  void setText(const std::string& s);

  void setViewport(float x, float y, float width, float height);

  osg::ref_ptr<osg::Camera> getHudCamera() const { return mHudCamera; }
  void setHudCamera(osg::ref_ptr<osg::Camera> v) { mHudCamera = v; }

  GLfloat getMiniAxesSize() const { return mMiniAxesSize; }
  void setMiniAxesSize(GLfloat v) { mMiniAxesSize = v; }

  osg::ref_ptr<zxd::GridFloor> getGridFloor() const { return mGridFloor; }
  void setGridFloor(osg::ref_ptr<zxd::GridFloor> v);

  osg::ref_ptr<osgText::Text> getText() const { return mText; }
  void setText(osg::ref_ptr<osgText::Text> v) { mText = v; }

  osg::ref_ptr<osg::Group> getRoot() const { return mRoot; }
  void setRoot(osg::ref_ptr<osg::Group> v) { mRoot = v; }

  ViewType getViewType() const { return mViewType; }
  void setViewType(ViewType v) { mViewType = v; }
};

class BlenderViewEventhandler : public osgGA::GUIEventHandler {
protected:
  BlenderView* mView;

protected:
  bool isCursorInView();

  virtual bool handle(
    const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
    switch (ea.getEventType()) {
      case osgGA::GUIEventAdapter::KEYDOWN:
        switch (ea.getKey()) {
          case osgGA::GUIEventAdapter::KEY_F1:
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
    return false;  // return true will stop event
  }
};

// bottom text information view
class TextView : public osgViewer::View {
protected:
  osg::ref_ptr<osgText::Text> mOpText;  // operation text
  osg::ref_ptr<osgText::Text> mPivotText;
  // osg::ref_ptr<osg::Camera> mHudCamera;
  osg::ref_ptr<osg::Group> mRoot;

public:
  TextView(osg::GraphicsContext* gc);

  osg::ref_ptr<osgText::Text> getOpText() const { return mOpText; }
  void setOpText(osg::ref_ptr<osgText::Text> v) { mOpText = v; }

  osg::ref_ptr<osgText::Text> getPivotText() const { return mPivotText; }
  void setPivotText(osg::ref_ptr<osgText::Text> v) { mPivotText = v; }

  void setViewport(float x, float y, float width, float height);

  // osg::ref_ptr<osg::Camera> getHudCamera() const { return mHudCamera; }
  // void setHudCamera(osg::ref_ptr<osg::Camera> v) { mHudCamera = v; }

  osg::ref_ptr<osg::Group> getRoot() const { return mRoot; }
  void setRoot(osg::ref_ptr<osg::Group> v) { mRoot = v; }
};
}

#endif /* BLENDERVIEW_H */
