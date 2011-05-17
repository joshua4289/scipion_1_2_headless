#include <jni.h>
#include "xmipp_ImageDouble.h"
#include "xmipp_ExceptionsHandler.h"
#include <data/image.h>
#include <data/fft.h>
#include "xmipp_InternalData.h"

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_storeIds
(JNIEnv *env, jclass cls) {
	peerId = env->GetFieldID(cls, "peer", "J");
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_create
(JNIEnv *env, jobject jobj) {
	Image<double> *image = new Image<double>();
	env->SetLongField(jobj, peerId, (long)image);
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_destroy
(JNIEnv *env, jobject jobj) {
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);
	delete image;
	image = NULL;
	env->SetLongField(jobj, peerId, (long)image);
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_read_1image
(JNIEnv *env, jobject jobj, jstring filename, jboolean read_data, jlong nimage) {
	std::string msg = "";
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
		try {
			const char *fnStr = env->GetStringUTFChars(filename, false);

			image->read(fnStr, (read_data)? DATA : HEADER, (size_t)nimage);
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Image is null";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_readApplyGeo(
		JNIEnv *env, jobject jimage, jobject jmetadata, jlong id) {
	std::string msg = "";
	Image<double> *image = GET_INTERNAL_IMAGE(jimage);
	MetaData *metadata = GET_INTERNAL_METADATA(jmetadata);

	if (image != NULL) {
		if (metadata != NULL) {
			try {
				image->readApplyGeo(*metadata, (long) id);
			} catch (XmippError xe) {
				msg = xe.getDefaultMessage();
			} catch (std::exception& e) {
				msg = e.what();
			} catch (...) {
				msg = "Unhandled exception";
			}
		} else {
			msg = "Metadata is null";
		}
	} else {
		msg = "Image is null";
	}

	// If there was an exception, sends it to java environment.
	if (!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_read_1preview
(JNIEnv *env, jobject jobj, jstring filename, jint w, jint h, jint slice, jlong nimage) {
	std::string msg = "";
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
		try {
			const char *fnStr = env->GetStringUTFChars(filename, false);

			image->readPreview(fnStr, (int)w, (int)h, (int)slice, (size_t)nimage);
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Image is null";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_setData__III_3D
(JNIEnv *env, jobject jobj, jint w, jint h, jint d, jdoubleArray data) {
	std::string msg = "";
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
		try {
			image->data.resizeNoCopy((int) d, (int) h, (int) w);
			env->GetDoubleArrayRegion(data, 0, w * h * d, MULTIDIM_ARRAY(image->data));
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Image is null";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_setData__IIII_3D
(JNIEnv *env, jobject jobj, jint width, jint height, jint depth, jint numberOfSlices, jdoubleArray data) {
	std::string msg = "";
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
		try {
			image->data.resize(numberOfSlices,depth,height,width,false);
			env->GetDoubleArrayRegion(data, 0, width * height * depth * numberOfSlices, MULTIDIM_ARRAY(image->data));
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Image is null";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_write
  (JNIEnv *env, jobject jobj, jstring filename, jint select_img, jboolean istack, jint mode, jint castWriteMode) {
	std::string msg = "";
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
		try {
			const char * fnStr = env->GetStringUTFChars(filename, false);

			image->write(fnStr,select_img,istack,mode,(CastWriteMode)castWriteMode);
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Image is null";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT jdoubleArray JNICALL Java_xmipp_ImageDouble_getData(JNIEnv *env,
		jobject jobj) {
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);
	if (image != NULL) {
		size_t size = image->getSize();
		jdoubleArray array = env->NewDoubleArray(size);
		env->SetDoubleArrayRegion(array, 0, size, MULTIDIM_ARRAY(image->data));
		return array;
	}

	return (jdoubleArray) NULL;
}

JNIEXPORT jdouble JNICALL Java_xmipp_ImageDouble_getPixel(JNIEnv *env,
		jobject jobj, jint x, jint y) {
	double value = 0;
	Image<double> * image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
		value = IMGPIXEL(*image, (int) x, (int) y);
	}

	return value;
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_setPixel
(JNIEnv *env, jobject jobj, jint x, jint y, jdouble value) {
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
	  IMGPIXEL(*image, (int) x, (int) y) = (double)value;
	}
}

JNIEXPORT jdouble JNICALL Java_xmipp_ImageDouble_getVoxel(JNIEnv *env,
		jobject jobj, jint x, jint y, jint z) {
	double value = 0;
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
	  value = A3D_ELEM(image->data,  (int) x, (int) y, (int) z);
	}

	return value;
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_setVoxel
(JNIEnv *env, jobject jobj, jint x, jint y, jint z, jdouble value) {
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
	  A3D_ELEM(image->data,  (int) x, (int) y, (int) z) = (double)value;
	}
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_convertPSD(JNIEnv *env, jobject jobj, jboolean useLogarithm) {
	std::string msg = "";
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if(image != NULL) {
		try {
			MultidimArray<double> out(image->getSize());
			xmipp2PSD(image->data, out, useLogarithm);
			image->data.clear(); // Frees memory.
			image->data = out;
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Image is NULL";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT jint JNICALL Java_xmipp_ImageDouble_getXsize(JNIEnv * env,
		jobject jobj) {
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
		return XSIZE(image->data);
	}

	return 0;
}

JNIEXPORT jint JNICALL Java_xmipp_ImageDouble_getYsize(JNIEnv * env,
		jobject jobj) {
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
		return YSIZE(image->data);
	}

	return 0;
}

JNIEXPORT jint JNICALL Java_xmipp_ImageDouble_getZsize(JNIEnv * env,
		jobject jobj) {
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
		return ZSIZE(image->data);
	}

	return 0;
}

JNIEXPORT jlong JNICALL Java_xmipp_ImageDouble_getNsize(JNIEnv *env,
		jobject jobj) {
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if (image != NULL) {
		return NSIZE(image->data);
	}

	return 0;
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_setXmippOrigin
(JNIEnv *env, jobject jobj) {
	std::string msg = "";
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);

	if(image != NULL) {
		try {
			image->data.setXmippOrigin();
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Image is NULL";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT void JNICALL Java_xmipp_ImageDouble_printShape
(JNIEnv *env, jobject jobj) {
	Image<double> *image = GET_INTERNAL_IMAGE(jobj);
	std::cout << (*image) << std::endl;
}
