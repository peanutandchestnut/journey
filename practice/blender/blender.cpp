/*
 * basic blender grab rotate scale
 */
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osgFX/Outline>
#include "auxiliary.h"
#include <osgGA/OrbitManipulator>
#include <osgUtil/IntersectVisitor>
#include <osgUtil/LineSegmentIntersector>
#include <osg/ValueObject>
#include "blendermanipulator.h"
#include "objectoperation.h"
#include "axisconstrainer.h"
#include "blenderobject.h"
#include "common.h"
#include "gridfloor.h"

osg::Camera* camera;
osg::Camera* hudCamera;
osg::ref_ptr<zxd::ObjectOperation> currentOperation;
osg::ref_ptr<zxd::BlenderObject> currentObject;
osg::ref_ptr<osg::Group> root;
osg::ref_ptr<zxd::GridFloor> gridFloor;
osg::ref_ptr<osgText::Font> font;
osg::ref_ptr<osgText::Text> opText;  // operation text
osg::ref_ptr<zxd::Cursor> cursor;
osgViewer::Viewer* viewer;

GLfloat fontSize = 12.0f;

osg::Vec4 outlineColor = osg::Vec4(1.0f, 0.5f, 0.0f, 1.0f);

GLuint selectMask = 1 << 0;

class BlendGuiEventhandler : public osgGA::GUIEventHandler {
protected:
  osg::ref_ptr<zxd::ObjectOperation> operation;

protected:
  void clearRotation() {
    // clear orientation
    osg::Vec3 translation;
    osg::Quat rotation;
    osg::Vec3 scale;
    osg::Quat so;
    currentObject->getMatrix().decompose(translation, rotation, scale, so);
    currentObject->setMatrix(
      osg::Matrix::scale(scale) * osg::Matrix::translate(translation));
  }
  void clearTranslation() {
    // clear translation
    const osg::Matrix& m = currentObject->getMatrix();
    currentObject->postMult(osg::Matrix::translate(-m.getTrans()));
  }
  void clearScale() {
    osg::Vec3 translation;
    osg::Quat rotation;
    osg::Vec3 scale;
    osg::Quat so;
    currentObject->getMatrix().decompose(translation, rotation, scale, so);
    currentObject->setMatrix(
      osg::Matrix::rotate(rotation) * osg::Matrix::translate(translation));
  }

  void selectObject(const osgGA::GUIEventAdapter& ea) {
    // select current object
    osgUtil::IntersectionVisitor iv;
    osg::ref_ptr<osgUtil::LineSegmentIntersector> lsi =
      new osgUtil::LineSegmentIntersector(
        osgUtil::Intersector::WINDOW, ea.getX(), ea.getY());
    iv.setIntersector(lsi);
    iv.setTraversalMask(selectMask);

    camera->accept(iv);

    if (lsi->containsIntersections()) {
      const osgUtil::LineSegmentIntersector::Intersection& result =
        *lsi->getIntersections().begin();

      for (GLuint i = 0; i < result.nodePath.size(); ++i) {
        osg::Node* node = result.nodePath[i];
        bool isBLenderObject = false;
        if (node->getUserValue("BlenderObject", isBLenderObject)) {
          if (currentObject && currentObject != node) currentObject->deselect();
          currentObject = static_cast<zxd::BlenderObject*>(node);
          currentObject->select(camera);
          break;
        }
      }

    } else {
      // do nothing if nothing hitten
      // currentObject->deselect();
    }
  }

  void placeCursor(const osgGA::GUIEventAdapter& ea) {
    osg::ref_ptr<osgUtil::LineSegmentIntersector> lsi =
      new osgUtil::LineSegmentIntersector(
        osgUtil::Intersector::WINDOW, ea.getX(), ea.getY());
    osgUtil::IntersectionVisitor iv(lsi);
    camera->accept(iv);

    if (lsi->containsIntersections()) {
      const osgUtil::LineSegmentIntersector::Intersection& intersection =
        *lsi->getIntersections().begin();

      // get intersection point in view space
      osg::Vec3 pos = intersection.localIntersectionPoint *
                      osg::computeLocalToWorld(intersection.nodePath);
      cursor->setMatrix(osg::Matrix::translate(pos));
    } else {
      const osg::Matrix& matView = camera->getViewMatrix();
      // get original cursor position in view space
      osg::Vec3 origPos = cursor->getMatrix().getTrans() * matView;

      // get camera ray intersection with near plane
      osg::Matrix matrix;
      if (camera->getViewport())
        matrix.preMult(camera->getViewport()->computeWindowMatrix());
      matrix.preMult(camera->getProjectionMatrix());
      matrix.invert(matrix);

      // point on near plane in view space
      osg::Vec3 p = osg::Vec3(ea.getX(), ea.getY(), 0) * matrix;

      // scale according z to place p at plane that contain origin cursor
      p *= origPos.z() / p.z();

      // now get final world position
      osg::Matrix matInvView = osg::Matrix::inverse(matView);

      cursor->setMatrix(osg::Matrix::translate(p * matInvView));
    }
  }

  virtual bool handle(
    const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
    switch (ea.getEventType()) {
      case osgGA::GUIEventAdapter::KEYDOWN:
        switch (ea.getKey()) {
          case osgGA::GUIEventAdapter::KEY_G:
            if (currentObject &&
                ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT) {
              clearTranslation();
            } else {
              createNewOperation<zxd::GrabOperation>(ea);
            }
            break;
          case osgGA::GUIEventAdapter::KEY_S:
            if (currentObject &&
                ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT) {
              clearScale();
            } else {
              createNewOperation<zxd::ScaleOperation>(ea);
            }
            break;
          case osgGA::GUIEventAdapter::KEY_R:
            if (currentObject &&
                ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT) {
              clearRotation();
            } else {
              if (currentOperation) {
                osg::ref_ptr<zxd::RotateOperation> ro =
                  osg::dynamic_pointer_cast<zxd::RotateOperation>(
                    currentOperation);
                if (ro) {
                  ro->setMode(ro->getMode() ^ 1);
                  return false;
                }
              }
              createNewOperation<zxd::RotateOperation>(ea);
            }
            break;
          default:
            break;
        }
        break;
      default:
        break;

      case osgGA::GUIEventAdapter::PUSH:

        if (currentOperation) {
          // operation confirm or cancel
          if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) {
            OSG_NOTICE << "confirm operation" << std::endl;
            currentOperation->confirm();
          } else {
            currentOperation->cancel();
            OSG_NOTICE << "cancel operation" << std::endl;
          }
          viewer->removeEventHandler(currentOperation);
          currentOperation = osg::ref_ptr<zxd::ObjectOperation>();

        } else {
          // select object
          if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) {
            selectObject(ea);
          } else if (ea.getButton() ==
                     osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) {
            // re place cursor
            placeCursor(ea);
          }
        }
    }
    return false;  // return true will stop event
  }

  template <typename T>
  void createNewOperation(const osgGA::GUIEventAdapter& ea) {
    if (!currentObject) return;

    OSG_NOTICE << "create new operation " << std::endl;

    if (currentOperation) {
      // do nothing if curreng operation is the same tyep as T
      if (dynamic_cast<T*>(currentOperation.get()) != NULL) return;

      OSG_NOTICE << "reuse last constrainer " << std::endl;

      // create new operation, use current axis constrainer
      osg::ref_ptr<zxd::AxisConstrainer> ac =
        currentOperation->getAxisConstrainer();

      currentOperation->cancel();

      currentOperation = new T();
      currentOperation->setAxisConstrainer(ac);

    } else {
      currentOperation = new T();
      currentOperation->setAxisConstrainer(new zxd::AxisConstrainer());
    }
    currentOperation->setTarget(currentObject);
    currentOperation->setInfoText(opText);
    currentOperation->init(camera, osg::Vec2(ea.getX(), ea.getY()));

    zxd::AxisConstrainer* ac = currentOperation->getAxisConstrainer();
    ac->setNodeMask(-1 & ~selectMask);
    root->addChild(ac);
    OSG_NOTICE << "add constrainer to scene" << std::endl;
    osg::Node* handle = currentOperation->getHandle();
    if (handle) {
      handle->setNodeMask(-1 & ~selectMask);
      hudCamera->addChild(handle);
      OSG_NOTICE << "add handle to hud camera" << std::endl;
    }

    viewer->addEventHandler(currentOperation);
  }
};

int main(int argc, char* argv[]) {
  osg::ArgumentParser ap(&argc, argv);
  ap.getApplicationUsage()->setApplicationName(ap.getApplicationName());
  ap.getApplicationUsage()->setCommandLineUsage(
    ap.getApplicationName() + " [options] filename ...");

  // load object
  osg::ref_ptr<osg::Node> node = osgDB::readNodeFiles(ap);
  if (!node) node = osgDB::readNodeFile("cessna.osg");

  font = osgText::readFontFile("fonts/arial.ttf");
  opText = zxd::createText(
    font, osg::Vec3(10.0f, fontSize + 10.0f, 0.0f), "", fontSize);

  root = new osg::Group;

  osg::ref_ptr<zxd::BlenderObject> bo = new zxd::BlenderObject();
  bo->getModel()->addChild(node);
  root->addChild(bo);

  osgViewer::Viewer v;
  viewer = &v;
  osg::ref_ptr<osgViewer::StatsHandler> sh = new osgViewer::StatsHandler();
  sh->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_I);
  // sh->setKeyEventPrintsOutStats(osgGA::GUIEventAdapter::KEY_J);
  // sh->setKeyEventPrintsOutStats('i');
  viewer->addEventHandler(sh);
  viewer->setSceneData(root);
  osg::ref_ptr<zxd::BlenderManipulator> manipulator =
    new zxd::BlenderManipulator();
  viewer->setCameraManipulator(manipulator);
  camera = viewer->getCamera();

  // camera->setCullingMode(
  // camera->getCullingMode() & ~osg::CullSettings::SMALL_FEATURE_CULLING);
  camera->setCullingMode(osg::CullSettings::NO_CULLING);

  hudCamera = zxd::createHUDCamera();

  cursor = new zxd::Cursor();
  root->addChild(cursor);

  osg::ref_ptr<osg::Geode> opLeaf = new osg::Geode();
  opLeaf->addDrawable(opText);
  hudCamera->addChild(opLeaf);
  root->addChild(hudCamera);

  gridFloor = new zxd::GridFloor();
  gridFloor->setScale(5);
  gridFloor->rebuild();
  gridFloor->setNodeMask(-1 & ~selectMask);
  root->addChild(gridFloor);

  viewer->addEventHandler(new BlendGuiEventhandler);

  return viewer->run();
}
