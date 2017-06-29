#include "blendermanipulator.h"
#include "zmath.h"
#include <cassert>
#include "blenderobject.h"
#include "blender.h"
#include <osg/ComputeBoundsVisitor>

namespace zxd {

//------------------------------------------------------------------------------
void BlenderManipulator::BlenderAnimationData::init(
  zxd::BlenderManipulator* om) {
  if (!mMotion) mMotion = new osgAnimation::InOutCubicMotion();
  mMotion->reset();
  mSrcYaw = mDestYaw = om->mYaw;
  mSrcPitch = mDestPitch = om->mPitch;
  mSrcDistance = mDestDistance = om->_distance;
  mSrcCenter = mDestCenter = om->_center;
}

//------------------------------------------------------------------------------
bool BlenderManipulator::handleKeyDown(
  const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) {
  switch (ea.getUnmodifiedKey()) {
    case osgGA::GUIEventAdapter::KEY_KP_End: {
      if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)
        viewBack(ea);
      else
        viewFront(ea);
    } break;

    case osgGA::GUIEventAdapter::KEY_KP_Page_Down: {
      if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)
        viewRight(ea);
      else
        viewLeft(ea);
    } break;

    case osgGA::GUIEventAdapter::KEY_KP_Home: {
      if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)
        viewBottom(ea);
      else
        viewTop(ea);
    } break;

    case osgGA::GUIEventAdapter::KEY_KP_Page_Up: {
      viewInverse(ea);
    } break;

    case osgGA::GUIEventAdapter::KEY_KP_Left: {
      if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)
        panModel(-mPanStep * -0.3f * _distance, 0);
      else
        yaw(mRotateStep);
    } break;
    case osgGA::GUIEventAdapter::KEY_KP_Right: {
      if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)
        panModel(mPanStep * -0.3f * _distance, 0);
      else
        yaw(-mRotateStep);
    } break;

    case osgGA::GUIEventAdapter::KEY_KP_Up: {
      if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)
        panModel(0, mPanStep * -0.3f * _distance);
      else
        pitch(mRotateStep);
    } break;
    case osgGA::GUIEventAdapter::KEY_KP_Down: {
      if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)
        panModel(0, -mPanStep * -0.3f * _distance);
      else
        pitch(-mRotateStep);
    } break;

    case osgGA::GUIEventAdapter::KEY_Home:
      home(ea, us);
      break;

    case osgGA::GUIEventAdapter::KEY_KP_Delete: {
      // zoom to selection
      zxd::BlenderObject* bo = mBlender->getCurObject();
      if (bo) {
        // osg::ComputeBoundsVisitor cbVisitor;
        // bo->accept(cbVisitor);
        // osg::BoundingSphere bs;
        // bs.expandRadiusBy(cbVisitor.getBoundingBox());

        // what's the difference between this and the codes above ?
        const osg::BoundingSphere& bs = bo->getBound();

        float fovy, aspectRatio, zNear, zFar;
        mBlender->getCamera()->getProjectionMatrix().getPerspective(
          fovy, aspectRatio, zNear, zFar);

        float destDistance =
          bs.radius() / sinf(osg::DegreesToRadians(fovy) * 0.5f);
        animPanZoom(destDistance, bo->getMatrix().getTrans(), ea.getTime());
      }
    } break;

    case osgGA::GUIEventAdapter::KEY_KP_Begin: //this is 5 in key pad
      toggleProjectionType();
      break;

    default:
      break;
  }
  return false;
}

//------------------------------------------------------------------------------
bool BlenderManipulator::performMovementMiddleMouseButton(
  const double eventTimeDelta, const double dx, const double dy) {
  // check shift status
  if (_ga_t0->getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT) {
    // pan model
    float scale = -0.3f * _distance;
    panModel(dx * scale, dy * scale);
  } else {
    // rotate
    osg::Vec2 offset(
      _ga_t0->getX() - _ga_t1->getX(), _ga_t0->getY() - _ga_t1->getY());
    offset *= 0.01f;
    mYaw -= offset.x();
    mPitch += offset.y();
    updateRotation();
  }

  return true;
}

//------------------------------------------------------------------------------
void BlenderManipulator::setByMatrix(const osg::Matrixd& matrix) {
  _center = osg::Vec3d(0., 0., -_distance) * matrix;
  // get yaw and pitch

  osg::Vec3 xAxis(matrix(0, 0), matrix(0, 1), matrix(0, 2));
  osg::Vec3 yAxis(matrix(1, 0), matrix(1, 1), matrix(1, 2));
  osg::Vec3 zAxis(matrix(2, 0), matrix(2, 1), matrix(2, 2));

  mYaw = atan2(xAxis[1], xAxis[0]);
  mPitch = zxd::Math::angleSigned(osg::Z_AXIS, zAxis, osg::X_AXIS);
  updateRotation();
}

//------------------------------------------------------------------------------
void BlenderManipulator::setTransformation(
  const osg::Vec3d& eye, const osg::Quat& rotation) {
  setByMatrix(osg::Matrix::rotate(rotation) * osg::Matrix::translate(eye));
}

//------------------------------------------------------------------------------
void BlenderManipulator::setTransformation(
  const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up) {
  _distance = (eye - center).length();
  osg::Matrix view = osg::Matrix::lookAt(eye, center, up);
  setByMatrix(osg::Matrix::inverse(view));
}

//------------------------------------------------------------------------------
void BlenderManipulator::applyAnimationStep(
  const double currentProgress, const double prevProgress) {
  auto ad = getBlenderAnimData();
  assert(ad);
  ad->mMotion->setTime(currentProgress);
  float f = ad->mMotion->getValue();

  mYaw = zxd::Math::linearInterpolate(ad->mSrcYaw, ad->mDestYaw, f);
  mPitch = zxd::Math::linearInterpolate(ad->mSrcPitch, ad->mDestPitch, f);
  _distance =
    zxd::Math::linearInterpolate(ad->mSrcDistance, ad->mDestDistance, f);
  _center = zxd::Math::linearInterpolate(ad->mSrcCenter, ad->mDestCenter, f);

  // clamp to 0 -2p
  if (currentProgress == 1) {
    clampAngle();
  }

  updateRotation();
}

//------------------------------------------------------------------------------
void BlenderManipulator::updateRotation() {
  /*
   * pitch along view x(local x), yaw along wolrd z
   *
   * so q0 = p0 * I * y0
   *    q1 = p1 * q0 * y1
   *       = p0 * p1 * q0 * y0 * y1
   *  .......
   *    q = p * y
   *  so pitch and yaw an be accumulated
   */

  _rotation = osg::Quat(mPitch, osg::X_AXIS) * osg::Quat(mYaw, osg::Z_AXIS);
  updateText();
}

//------------------------------------------------------------------------------
void BlenderManipulator::updateText() {
  // update text

  std::string projType, rotType("User");

  double left, right, top, bottom, near, far;
  bool isPerspective = mBlender->getCamera()->getProjectionMatrixAsFrustum(
    left, right, top, bottom, near, far);
  projType = isPerspective ? "Persp" : "Ortho";

  if (zxd::Math::isAboutf(mPitch, osg::PI_2)) {
    if (zxd::Math::isAboutf(mYaw, 0))
      rotType = "Front";
    else if (zxd::Math::isAboutf(mYaw, osg::PI_2))
      rotType = "Right";
    else if (zxd::Math::isAboutf(mYaw, osg::PI))
      rotType = "Back";
    else if (zxd::Math::isAboutf(mYaw, osg::PI_2 * 3))
      rotType = "Left";
  } else if (zxd::Math::isAboutf(mYaw, 0.0f)) {
    if (zxd::Math::isAboutf(mPitch, 0))
      rotType = "Up";
    else if (zxd::Math::isAboutf(mPitch, osg::PI))
      rotType = "bottom";
  }

  mViewText->setText(rotType + " " + projType);
}

//------------------------------------------------------------------------------
void BlenderManipulator::viewInverse(const osgGA::GUIEventAdapter& ea) {
  static float epsilon = 0.001f;

  auto ad = getBlenderAnimData();
  float destPitch, destYaw;

  // get current pitch including animation
  float curDestPitch = ad->_isAnimating ? ad->mDestPitch : mPitch;
  float curDestYaw = ad->_isAnimating ? ad->mDestYaw : mYaw;

  float absmod = std::abs(std::fmod(curDestPitch, osg::PI));

  if (absmod <= epsilon || osg::PI - absmod <= epsilon) {
    // pitch 180 only if it's top or bottom view
    destPitch = curDestPitch + osg::PI;
    destYaw = curDestYaw;
  } else {
    destPitch = curDestPitch;
    destYaw = curDestYaw + osg::PI;
  }

  animRotView(destPitch, destYaw, ea.getTime());
}

//------------------------------------------------------------------------------
void BlenderManipulator::yaw(GLfloat v) {
  mYaw += v;
  clampAngle();
  updateRotation();
}

//------------------------------------------------------------------------------
void BlenderManipulator::pitch(GLfloat v) {
  mPitch += v;
  clampAngle();
  updateRotation();
}

//------------------------------------------------------------------------------
void BlenderManipulator::toggleProjectionType() {
  // becareful fovy not in radian
  osg::Camera* camera = mBlender->getCamera();

  // becareful fovy not in radian
  double fovy, aspectRatio, zNear, zFar;
  if (camera->getProjectionMatrixAsPerspective(
        fovy, aspectRatio, zNear, zFar)) {
    mPerspFovy = fovy;
    mPerspAspectRatio = aspectRatio;
    mPerspZNear = zNear;
    mPerspZFar = zFar;

    camera->setProjectionMatrixAsOrtho(
      mOrthoLeft, mOrthoRight, mOrthoTop, mOrthoBottom, mOrthoNear, mOrthoFar);

  } else {
    double left, right, top, bottom, near, far;
    if (camera->getProjectionMatrixAsOrtho(
          left, right, top, bottom, near, far)) {
      mOrthoLeft = left;
      mOrthoRight = right;
      mOrthoTop = top;
      mOrthoBottom = bottom;
      mOrthoNear = near;
      mOrthoFar = far;

      camera->setProjectionMatrixAsPerspective(
        mPerspFovy, mPerspAspectRatio, mPerspZNear, mPerspZFar);
    }
  }

  updateText();
}

//------------------------------------------------------------------------------
void BlenderManipulator::setBlender(Blender* v) {
  mBlender = v;
  setViewText(mBlender->getViewText());

  osg::Camera* camera = mBlender->getCamera();

  osg::DisplaySettings* ds = osg::DisplaySettings::instance();
  double height = ds->getScreenHeight();
  double width = ds->getScreenWidth();
  double distance = ds->getScreenDistance();  // what's this
  double vfov = osg::RadiansToDegrees(atan2(height / 2.0f, distance) * 2.0);

  if (!camera->getProjectionMatrixAsPerspective(
        mPerspFovy, mPerspAspectRatio, mPerspZNear, mPerspZFar)) {
    mPerspFovy = vfov;
    mPerspAspectRatio = width / height;
    mPerspZNear = 1.0f;
    mPerspZFar = 10000.0f;
  }

  if (!camera->getProjectionMatrixAsOrtho(mOrthoLeft, mOrthoRight, mOrthoTop,
        mOrthoBottom, mOrthoNear, mOrthoFar)) {
    mOrthoLeft = -width * 0.5f;
    mOrthoRight = width * 0.5f;
    mOrthoBottom = -height * 0.5f;
    mOrthoTop = height * 0.5f;
    mOrthoNear = -10000.0f;
    mOrthoFar = 10000.0f;
  }
}
}
