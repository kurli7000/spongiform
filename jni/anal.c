/////////////////////////////////////////////////////////////////////
// ANAL2.0b
// Android Native App Library
// (c) Kewlers 2012

#include "anal.h"
#include <assert.h>
#include <jni.h>

#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

// bass and rocket
#include "sync/sync.h"
#include "sync/base.h"
#include "bass.h"
#include "time.h"

// image loader
#define STBI_NO_STDIO
#define STBI_NO_HDR
#include "stb_image.c"

#ifdef DISKWRITE
#include "writebmp.c"
#endif



/////////////////////////////////////////////////////////////////////
// GLOBALS

#define rpb 8.0 /* rows per beat */
double row_rate = ((double)(BPM) / 60.0) * rpb;
struct sync_device *rocket;

const struct sync_track *sync_camdist;
const struct sync_track *sync_camrot;
const struct sync_track *sync_camfov;
const struct sync_track *sync_camxoffs;
const struct sync_track *sync_camyoffs;
const struct sync_track *sync_camzoffs;
const struct sync_track *sync_camxtgt;
const struct sync_track *sync_camytgt;
const struct sync_track *sync_camztgt;
const struct sync_track *sync_camroll;

ROCKET_VARIABLES

HSTREAM stream;
static long timerdelta;

int currentshader = 0;
float analaspect = 1.0;
int anal_xres, anal_yres;

float black[]={0.0, 0.0, 0.0, 1.0};
float white[]={1.0, 1.0, 1.0, 1.0};

/////////////////////////////////////////////////////////////////////
// STUFF

#define pi 3.1415926536
#define ONCE(x) do { static int da_onceflag=0; if (da_onceflag==0) { x; da_onceflag=1; } } while (0)
int anal_seed;
#define JMAGIC 69069
#define KMAGIC 0x1
#define random() ((unsigned)(anal_seed = (anal_seed*JMAGIC+KMAGIC) & 0xffffffff)>>17)
#define ranf() ((unsigned)(anal_seed = (anal_seed*JMAGIC+KMAGIC) & 0xffffffff)*(1./4292967296.))
#define ranb() ((unsigned)(anal_seed = (anal_seed*JMAGIC+KMAGIC) & 0xffffffff)>>24)
#define clamp(x, a, b) ((x)<(a)?(a):(x)>(b)?(b):(x))
#define dot(a, b) ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define normalize(d) { float r=1.0/sqrt(dot((d), (d))); (d)[0]*=r; (d)[1]*=r; (d)[2]*=r; }
#define r0(a) ((a)<0?0:(a))
#define vlen(a) (sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]))

void cross(float *tgt, float *a, float *b) {
	tgt[0] = a[1]*b[2]-a[2]*b[1];  
	tgt[1] = a[2]*b[0]-a[0]*b[2];
	tgt[2] = a[0]*b[1]-a[1]*b[0];
}

void resizeanal(int w, int h) {
	anal_xres = w;
	anal_yres = h;
	analaspect = (float)w/(float)h;
    glViewport(0, 0, w, h);
}

void initanal() {
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
}

void clear(float *col) {
    glClearColor(col[0], col[1], col[2], col[3]);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}


/////////////////////////////////////////////////////////////////////
// ROCKET


float SYNC(const struct sync_track *t, double row) {
	if (t) return sync_get_val(t, row);
    else return 0.0;
}

int SYNCi(const struct sync_track *t, double row) {
	return (int)floor(SYNC(t, row));
}

void SYNC2(float *buf, const struct sync_track *t1, const struct sync_track *t2, double row) {
	buf[0]=SYNC(t1, row);
	buf[1]=SYNC(t2, row);
}

void SYNC3(float *buf, const struct sync_track *t1, const struct sync_track *t2, const struct sync_track *t3, double row) {
	buf[0]=SYNC(t1, row);
	buf[1]=SYNC(t2, row);
	buf[2]=SYNC(t3, row);
}

void bass_pause(void *d, int flag) {
	if (flag) BASS_ChannelPause((HSTREAM)d);
	else BASS_ChannelPlay((HSTREAM)d, FALSE);
}

#ifdef DISKWRITE
#ifdef SYNC_PLAYER
void bass_set_row(void *d, int row) {
	QWORD pos = BASS_ChannelSeconds2Bytes((HSTREAM)d, row / row_rate);
	BASS_ChannelSetPosition((HSTREAM)d, pos, BASS_POS_BYTE);
}
#endif
#endif

#ifndef SYNC_PLAYER

double bass_get_row(HSTREAM h) {
	QWORD pos = BASS_ChannelGetPosition(h, BASS_POS_BYTE);
	double time = BASS_ChannelBytes2Seconds(h, pos);
//	return time * row_rate;
	return time * row_rate + 0.001;
}

void bass_set_row(void *d, int row) {
	QWORD pos = BASS_ChannelSeconds2Bytes((HSTREAM)d, row / row_rate);
	BASS_ChannelSetPosition((HSTREAM)d, pos, BASS_POS_BYTE);
}

int bass_is_playing(void *d) {
	return BASS_ChannelIsActive((HSTREAM)d) == BASS_ACTIVE_PLAYING;
}

struct sync_cb bass_cb = {
	bass_pause,
	bass_set_row,
	bass_is_playing
};

#endif /* !defined(SYNC_PLAYER) */



/////////////////////////////////////////////////////////////////////
// ASSETS

static AAssetManager *assmgr;
/*
typedef struct {
  unsigned char *data;
  unsigned int size;
} Assetloader;
*/

Assetloader loadasset(char *filename) {
	Assetloader as;
    AAsset *asset = AAssetManager_open(assmgr, filename, AASSET_MODE_UNKNOWN);
    assert(asset != NULL);

    char *buff = (char*) AAsset_getBuffer(asset);
    assert(buff != NULL);

    as.size = AAsset_getLength(asset);
    as.data = malloc(as.size);
    memcpy(as.data, buff, as.size);

    AAsset_close(asset);
    
    return as;
}

/////////////////////////////////////////////////////////////////////
// LOGGING

static void printglstring(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkerror(const char* op) {
#ifndef FINAL
	GLint error;
    for (error = glGetError(); error; error = glGetError()) {
    	if (error==GL_NO_ERROR) LOGI("after %s() glError GL_NO_ERROR (0x%x)\n", op, error);
    	else if (error==GL_INVALID_ENUM) LOGI("after %s() glError GL_INVALID_ENUM (0x%x)\n", op, error);
    	else if (error==GL_INVALID_VALUE) LOGI("after %s() glError GL_INVALID_VALUE (0x%x)\n", op, error);
    	else if (error==GL_INVALID_OPERATION) LOGI("after %s() glError GL_INVALID_OPERATION (0x%x)\n", op, error);
    	else if (error==GL_INVALID_FRAMEBUFFER_OPERATION) LOGI("after %s() glError GL_INVALID_FRAMEBUFFER_OPERATION (0x%x)\n", op, error);
    	else if (error==GL_OUT_OF_MEMORY) LOGI("after %s() glError GL_OUT_OF_MEMORY (0x%x)\n", op, error);
    	else LOGI("after %s() UNKNOWN glError (0x%x)\n", op, error);
    }
#endif
}

int checkext(char *extension) {
  char *extlist=(char *)glGetString(GL_EXTENSIONS);

  if (!extension || !extlist) return 0;

  while (*extlist) {
    unsigned int len=strcspn(extlist, " ");

    if (strlen(extension)==len && strncmp(extension, extlist, len)==0) return 1;
    extlist+=len+1;
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////
// BASS

Assetloader bass_biisi;

static long getmsec() {
	struct timeval paska;
	gettimeofday(&paska, NULL);
	return paska.tv_sec*1000+paska.tv_usec/1000;
}

#ifdef SYNC_PLAYER
static double bass_get_row() {
  return (getmsec()-timerdelta)/60000.0*BPM*rpb;
}
#endif // SYNC_PLAYER

BOOL init_bass() {

	bass_biisi = loadasset(biisi_filename);
	
	if (HIWORD(BASS_GetVersion())!=BASSVERSION) {
		LOGE("An incorrect version of BASS was loaded");
		return FALSE;
	}

	if (!BASS_Init(-1, 44100, 0, NULL, NULL)) {
        LOGE("BASS Can't init");
        return FALSE;
	}

    stream = BASS_StreamCreateFile(TRUE, bass_biisi.data, 0, bass_biisi.size, BASS_STREAM_PRESCAN);
	if (!stream) {
		LOGE("error opening tune");
		return FALSE;
	}

	// rocket
#ifdef SYNC_PLAYER
	rocket = sync_create_device("sync");
#else
	rocket = sync_create_device("/sdcard/tmp/sync");
#endif

	if (!rocket) {
		LOGE("error initializing rocket");
		return FALSE;
	}
    
#ifndef SYNC_PLAYER
	sync_set_callbacks(rocket, &bass_cb, (void *)stream);
	if (sync_connect(rocket, ROCKET_HOST, SYNC_DEFAULT_PORT)) {
        LOGE("failed to connect to host");
        return FALSE;
	} else {
		LOGI("rocket connection established");
	}
#endif

	sync_camdist = sync_get_track(rocket, "cam.dist");
	sync_camrot = sync_get_track(rocket, "cam.rot");
	sync_camfov = sync_get_track(rocket, "cam.fov");
	sync_camxoffs = sync_get_track(rocket, "cam.xoffs");
	sync_camyoffs = sync_get_track(rocket, "cam.yoffs");
	sync_camzoffs = sync_get_track(rocket, "cam.zoffs");
	sync_camxtgt = sync_get_track(rocket, "cam.xtgt");
	sync_camytgt = sync_get_track(rocket, "cam.ytgt");
	sync_camztgt = sync_get_track(rocket, "cam.ztgt");
	sync_camroll = sync_get_track(rocket, "cam.roll");

	ROCKET_TRACKS
	
	if (!BASS_Start()) return FALSE;
//	if (!BASS_ChannelPlay(stream, FALSE)) return FALSE;
	bass_pause((void *)stream, TRUE);
//	timerdelta = getmsec();
	
    LOGI("bass initialized");
    return TRUE;
}

void bass_startplaying() {
	bass_pause((void *)stream, FALSE);
	timerdelta = getmsec();
}

void close_bass() {
	BASS_StreamFree(stream);
	BASS_Free();
	free(bass_biisi.data);
	stream = 0;
	sync_destroy_device(rocket);
	rocket = 0;
	LOGI("bass closed");
}

/*
float gettime_laggy() {
	QWORD pos = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
	double time = BASS_ChannelBytes2Seconds(stream, pos);
	return time * (BPM / 60.0) / 8.0;
}
*/

/////////////////////////////////////////////////////////////////////
// TEXTURES

int loadtexture(char *filename, char *alphafilename) {
	int x, y, n;
	Assetloader al = loadasset(filename);
	int bpp = alphafilename?4:3;
	unsigned char *data = stbi_load_from_memory(al.data, al.size, &x, &y, &n, bpp);
	int tnum;

	if (alphafilename) {
		Assetloader al2 = loadasset(alphafilename);
		int ax, ay, an, i;
		unsigned char *adata = stbi_load_from_memory(al2.data, al2.size, &ax, &ay, &an, 3);
		if (x!=ax || y!=ay || n!=an) LOGE("alpha texture not same size as color!");
		for (i=0; i<ax*ay; i++) data[i*4+3]=adata[i*3];
		stbi_image_free(adata);
	    free(al2.data);
	}

	glGenTextures(1, &tnum);
	glBindTexture(GL_TEXTURE_2D, tnum);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, bpp==4?GL_RGBA:GL_RGB, x, y, 0, bpp==4?GL_RGBA:GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	stbi_image_free(data);
    free(al.data);

    if (alphafilename) LOGI("loaded texture %s / %s, %ix%ix%i", filename, alphafilename, x, y, 4);
    else LOGI("loaded texture %s, %ix%ix%i", filename, x, y, n);
    
    return tnum;
}


int loadcubemap(char *filename) {
	Assetloader al;
	unsigned char *data;
	int tnum;
	int x, y, n;
	char filename2[128];

	glGenTextures(1, &tnum);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tnum);
	
	sprintf(filename2, "%s-pos-x.jpg", filename);
	al = loadasset(filename2);
	data = stbi_load_from_memory(al.data, al.size, &x, &y, &n, 3);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
    free(al.data);
    LOGI("loaded cube map %s, %ix%ix%i", filename2, x, y, n);
    
	sprintf(filename2, "%s-neg-x.jpg", filename);
	al = loadasset(filename2);
	data = stbi_load_from_memory(al.data, al.size, &x, &y, &n, 3);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
    free(al.data);
    LOGI("loaded cube map %s, %ix%ix%i", filename2, x, y, n);

	sprintf(filename2, "%s-pos-y.jpg", filename);
	al = loadasset(filename2);
	data = stbi_load_from_memory(al.data, al.size, &x, &y, &n, 3);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
    free(al.data);
    LOGI("loaded cube map %s, %ix%ix%i", filename2, x, y, n);

	sprintf(filename2, "%s-neg-y.jpg", filename);
	al = loadasset(filename2);
	data = stbi_load_from_memory(al.data, al.size, &x, &y, &n, 3);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
    free(al.data);
    LOGI("loaded cube map %s, %ix%ix%i", filename2, x, y, n);
    
	sprintf(filename2, "%s-pos-z.jpg", filename);
	al = loadasset(filename2);
	data = stbi_load_from_memory(al.data, al.size, &x, &y, &n, 3);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
    free(al.data);
    LOGI("loaded cube map %s, %ix%ix%i", filename2, x, y, n);

	sprintf(filename2, "%s-neg-z.jpg", filename);
	al = loadasset(filename2);
	data = stbi_load_from_memory(al.data, al.size, &x, &y, &n, 3);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
    free(al.data);
    LOGI("loaded cube map %s, %ix%ix%i", filename2, x, y, n);
	
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	return tnum;
}


/////////////////////////////////////////////////////////////////////
// MATRIX STUFF


static float identitymatrix[16]=
		{1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0};

void identity(float *matrix) {
	memcpy(matrix, identitymatrix, sizeof(float)*16);
}

void multmatrix(float *result, float *matrix1, float *matrix2) {
	result[0]=matrix1[0]*matrix2[0]+matrix1[4]*matrix2[1]+matrix1[8]*matrix2[2]+matrix1[12]*matrix2[3];
	result[4]=matrix1[0]*matrix2[4]+matrix1[4]*matrix2[5]+matrix1[8]*matrix2[6]+matrix1[12]*matrix2[7];
	result[8]=matrix1[0]*matrix2[8]+matrix1[4]*matrix2[9]+matrix1[8]*matrix2[10]+matrix1[12]*matrix2[11];
	result[12]=matrix1[0]*matrix2[12]+matrix1[4]*matrix2[13]+matrix1[8]*matrix2[14]+matrix1[12]*matrix2[15];
	result[1]=matrix1[1]*matrix2[0]+matrix1[5]*matrix2[1]+matrix1[9]*matrix2[2]+matrix1[13]*matrix2[3];
	result[5]=matrix1[1]*matrix2[4]+matrix1[5]*matrix2[5]+matrix1[9]*matrix2[6]+matrix1[13]*matrix2[7];
	result[9]=matrix1[1]*matrix2[8]+matrix1[5]*matrix2[9]+matrix1[9]*matrix2[10]+matrix1[13]*matrix2[11];
	result[13]=matrix1[1]*matrix2[12]+matrix1[5]*matrix2[13]+matrix1[9]*matrix2[14]+matrix1[13]*matrix2[15];
	result[2]=matrix1[2]*matrix2[0]+matrix1[6]*matrix2[1]+matrix1[10]*matrix2[2]+matrix1[14]*matrix2[3];
	result[6]=matrix1[2]*matrix2[4]+matrix1[6]*matrix2[5]+matrix1[10]*matrix2[6]+matrix1[14]*matrix2[7];
	result[10]=matrix1[2]*matrix2[8]+matrix1[6]*matrix2[9]+matrix1[10]*matrix2[10]+matrix1[14]*matrix2[11];
	result[14]=matrix1[2]*matrix2[12]+matrix1[6]*matrix2[13]+matrix1[10]*matrix2[14]+matrix1[14]*matrix2[15];
	result[3]=matrix1[3]*matrix2[0]+matrix1[7]*matrix2[1]+matrix1[11]*matrix2[2]+matrix1[15]*matrix2[3];
	result[7]=matrix1[3]*matrix2[4]+matrix1[7]*matrix2[5]+matrix1[11]*matrix2[6]+matrix1[15]*matrix2[7];
	result[11]=matrix1[3]*matrix2[8]+matrix1[7]*matrix2[9]+matrix1[11]*matrix2[10]+matrix1[15]*matrix2[11];
	result[15]=matrix1[3]*matrix2[12]+matrix1[7]*matrix2[13]+matrix1[11]*matrix2[14]+matrix1[15]*matrix2[15];
}

void translatef(float *matrix, float x, float y, float z) {
	matrix[12]=matrix[0]*x+matrix[4]*y+matrix[8]*z+matrix[12];
	matrix[13]=matrix[1]*x+matrix[5]*y+matrix[9]*z+matrix[13];
	matrix[14]=matrix[2]*x+matrix[6]*y+matrix[10]*z+matrix[14];
	matrix[15]=matrix[3]*x+matrix[7]*y+matrix[11]*z+matrix[15];
}

void scalef(float *matrix, float x, float y, float z) {
	matrix[0]*=x;
	matrix[4]*=y;
	matrix[8]*=z;
	
	matrix[1]*=x;
	matrix[5]*=y;
	matrix[9]*=z;

	matrix[2]*=x;
	matrix[6]*=y;
	matrix[10]*=z;

	matrix[3]*=x;
	matrix[7]*=y;
	matrix[11]*=z;
}

void rotatef(float *matrix, float angleInRadians, float x, float y, float z) {
	float m[16], rotate[16];
	float OneMinusCosAngle, CosAngle, SinAngle;
	float A_OneMinusCosAngle, C_OneMinusCosAngle;
	CosAngle=cosf(angleInRadians);			//Some stuff for optimizing code
	OneMinusCosAngle=1.0-CosAngle;
	SinAngle=sinf(angleInRadians);
	A_OneMinusCosAngle=x*OneMinusCosAngle;
	C_OneMinusCosAngle=z*OneMinusCosAngle;
	//Make a copy
	m[0]=matrix[0];
	m[1]=matrix[1];
	m[2]=matrix[2];
	m[3]=matrix[3];
	m[4]=matrix[4];
	m[5]=matrix[5];
	m[6]=matrix[6];
	m[7]=matrix[7];
	m[8]=matrix[8];
	m[9]=matrix[9];
	m[10]=matrix[10];
	m[11]=matrix[11];
	m[12]=matrix[12];
	m[13]=matrix[13];
	m[14]=matrix[14];
	m[15]=matrix[15];

	rotate[ 0]=x*A_OneMinusCosAngle+CosAngle;
	rotate[ 1]=y*A_OneMinusCosAngle+z*SinAngle;
	rotate[ 2]=z*A_OneMinusCosAngle-y*SinAngle;
	rotate[ 3]=0.0;

	rotate[ 4]=y*A_OneMinusCosAngle-z*SinAngle;
	rotate[ 5]=y*y*OneMinusCosAngle+CosAngle;
	rotate[ 6]=y*C_OneMinusCosAngle+x*SinAngle;
	rotate[ 7]=0.0;

	rotate[ 8]=x*C_OneMinusCosAngle+y*SinAngle;
	rotate[ 9]=y*C_OneMinusCosAngle-x*SinAngle;
	rotate[10]=z*C_OneMinusCosAngle+CosAngle;
	rotate[11]=0.0;
	
	//The last column of rotate[] is {0 0 0 1}
	matrix[0]=m[0]*rotate[0]+m[4]*rotate[1]+m[8]*rotate[2];
	matrix[4]=m[0]*rotate[4]+m[4]*rotate[5]+m[8]*rotate[6];
	matrix[8]=m[0]*rotate[8]+m[4]*rotate[9]+m[8]*rotate[10];
	//matrix[12]=matrix[12];
	matrix[1]=m[1]*rotate[0]+m[5]*rotate[1]+m[9]*rotate[2];
	matrix[5]=m[1]*rotate[4]+m[5]*rotate[5]+m[9]*rotate[6];
	matrix[9]=m[1]*rotate[8]+m[5]*rotate[9]+m[9]*rotate[10];
	//matrix[13]=matrix[13];
	matrix[2]=m[2]*rotate[0]+m[6]*rotate[1]+m[10]*rotate[2];
	matrix[6]=m[2]*rotate[4]+m[6]*rotate[5]+m[10]*rotate[6];
	matrix[10]=m[2]*rotate[8]+m[6]*rotate[9]+m[10]*rotate[10];
	//matrix[14]=matrix[14];
	matrix[3]=m[3]*rotate[0]+m[7]*rotate[1]+m[11]*rotate[2];
	matrix[7]=m[3]*rotate[4]+m[7]*rotate[5]+m[11]*rotate[6];
	matrix[11]=m[3]*rotate[8]+m[7]*rotate[9]+m[11]*rotate[10];
	//matrix[15]=matrix[15];
}


void lookat(float *matrix, float *pos, float *tgt, float *yvec) {
	float forward[3], side[3], up[3];
	float matrix2[16], resmatrix[16];

	forward[0]=tgt[0]-pos[0];
	forward[1]=tgt[1]-pos[1];
	forward[2]=tgt[2]-pos[2];
	normalize(forward);

	cross(side, forward, yvec);
	normalize(side);
	cross(up, side, forward);

	matrix2[0]=side[0];
	matrix2[4]=side[1];
	matrix2[8]=side[2];
	matrix2[12]=0.0;

	matrix2[1]=up[0];
	matrix2[5]=up[1];
	matrix2[9]=up[2];
	matrix2[13]=0.0;

	matrix2[2]=-forward[0];
	matrix2[6]=-forward[1];
	matrix2[10]=-forward[2];
	matrix2[14]=0.0;

	matrix2[3]=matrix2[7]=matrix2[11]=0.0;
	matrix2[15]=1.0;

	multmatrix(resmatrix, matrix, matrix2);
	translatef(resmatrix, -pos[0], -pos[1], -pos[2]);
	
	memcpy(matrix, resmatrix, sizeof(float)*16);
}

void frustum(float *matrix, float left, float right, float bottom, float top, float znear, float zfar) {
	float matrix2[16], temp, temp2, temp3, temp4, resmatrix[16];
	temp=2.0*znear;
	temp2=right-left;
	temp3=top-bottom;
	temp4=zfar-znear;
	matrix2[0]=temp/temp2;
	matrix2[1]=0.0;
	matrix2[2]=0.0;
	matrix2[3]=0.0;
	matrix2[4]=0.0;
	matrix2[5]=temp/temp3;
	matrix2[6]=0.0;
	matrix2[7]=0.0;
	matrix2[8]=(right+left)/temp2;
	matrix2[9]=(top+bottom)/temp3;
	matrix2[10]=(-zfar-znear)/temp4;
	matrix2[11]=-1.0;
	matrix2[12]=0.0;
	matrix2[13]=0.0;
	matrix2[14]=(-temp*zfar)/temp4;
	matrix2[15]=0.0;
	multmatrix(resmatrix, matrix, matrix2);
	memcpy(matrix, resmatrix, sizeof(float)*16);
}

void perspective(float *matrix, float fov, float znear, float zfar) {
	float ymax, xmax;
	ymax=znear*tanf(fov*pi/360.0);
	xmax=ymax*analaspect;
	frustum(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
}

#define SWAP_ROWS_FLOAT(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

void inverse(float *out, float *m) {
	float wtmp[4][8];
	float m0, m1, m2, m3, s;
	float *r0, *r1, *r2, *r3;
	
	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

	r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
	r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
	r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
	r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
	r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
	r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
	r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
    r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
    r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
    r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
    r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
    r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

   /* choose pivot - or die */
    if (fabsf(r3[0]) > fabsf(r2[0])) SWAP_ROWS_FLOAT(r3, r2);
    if (fabsf(r2[0]) > fabsf(r1[0])) SWAP_ROWS_FLOAT(r2, r1);
	if (fabsf(r1[0]) > fabsf(r0[0])) SWAP_ROWS_FLOAT(r1, r0);
	if (0.0 == r0[0]) {
		LOGE("inverse failed");
		return;
	}
   	/* eliminate first variable     */
	m1 = r1[0] / r0[0];
	m2 = r2[0] / r0[0];
	m3 = r3[0] / r0[0];
   	s = r0[1];
   	r1[1] -= m1 * s;
   	r2[1] -= m2 * s;
   	r3[1] -= m3 * s;
   	s = r0[2];
   	r1[2] -= m1 * s;
   	r2[2] -= m2 * s;
   	r3[2] -= m3 * s;
   	s = r0[3];
   	r1[3] -= m1 * s;
   	r2[3] -= m2 * s;
   	r3[3] -= m3 * s;
   	s = r0[4];
   	if (s != 0.0) {
    	r1[4] -= m1 * s;
    	r2[4] -= m2 * s;
    	r3[4] -= m3 * s;
   	}
   	s = r0[5];
   	if (s != 0.0) {
    	r1[5] -= m1 * s;
    	r2[5] -= m2 * s;
    	r3[5] -= m3 * s;
   	}
   	s = r0[6];
   	if (s != 0.0) {
    	r1[6] -= m1 * s;
    	r2[6] -= m2 * s;
    	r3[6] -= m3 * s;
   	}
   	s = r0[7];
   	if (s != 0.0) {
    	r1[7] -= m1 * s;
    	r2[7] -= m2 * s;
      	r3[7] -= m3 * s;
   	}

   	/* choose pivot - or die */
   	if (fabsf(r3[1]) > fabsf(r2[1])) SWAP_ROWS_FLOAT(r3, r2);
	if (fabsf(r2[1]) > fabsf(r1[1])) SWAP_ROWS_FLOAT(r2, r1);
	if (0.0 == r1[1]) {
		LOGE("inverse failed");
      	return;
    }

   	/* eliminate second variable */
   	m2 = r2[1] / r1[1];
   	m3 = r3[1] / r1[1];
   	r2[2] -= m2 * r1[2];
   	r3[2] -= m3 * r1[2];
   	r2[3] -= m2 * r1[3];
   	r3[3] -= m3 * r1[3];
   	s = r1[4];
   	if (0.0 != s) {
      	r2[4] -= m2 * s;
      	r3[4] -= m3 * s;
   	}
   	s = r1[5];
   	if (0.0 != s) {
      	r2[5] -= m2 * s;
      	r3[5] -= m3 * s;
   	}
   	s = r1[6];
   	if (0.0 != s) {
      	r2[6] -= m2 * s;
      	r3[6] -= m3 * s;
  	}
   	s = r1[7];
   	if (0.0 != s) {
      	r2[7] -= m2 * s;
      	r3[7] -= m3 * s;
   	}

	/* choose pivot - or die */
	if (fabsf(r3[2]) > fabsf(r2[2])) SWAP_ROWS_FLOAT(r3, r2);
	if (0.0 == r2[2]) {
		LOGE("inverse failed");   
      	return;
	}
	/* eliminate third variable */
	m3 = r3[2] / r2[2];
	r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
	r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];

	/* last check */
	if (0.0 == r3[3]) {
		LOGE("inverse failed");
		return;
	}
	s = 1.0 / r3[3];		/* now back substitute row 3 */
	r3[4] *= s;
	r3[5] *= s;
	r3[6] *= s;
	r3[7] *= s;

	m2 = r2[3];			/* now back substitute row 2 */
	s = 1.0 / r2[2];
	r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
	r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
	m1 = r1[3];
	r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
	r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
	r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

	m1 = r1[2];			/* now back substitute row 1 */
	s = 1.0 / r1[1];
	r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
	r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
	m0 = r0[2];
	r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
	r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

	m0 = r0[1];			/* now back substitute row 0 */
	s = 1.0 / r0[0];
	r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
	r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

	MAT(out, 0, 0) = r0[4];
	MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
	MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
	MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
	MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
	MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
	MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
	MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
	MAT(out, 3, 3) = r3[7];
}


void transpose(float *result, float *m) {
	result[0]=m[0];
	result[1]=m[4];
	result[2]=m[8];
	result[3]=m[12];
	result[4]=m[1];
	result[5]=m[5];
	result[6]=m[9];
	result[7]=m[13];
	result[8]=m[2];
	result[9]=m[6];
	result[10]=m[10];
	result[11]=m[14];
	result[12]=m[3];
	result[13]=m[7];
	result[14]=m[11];
	result[15]=m[15];
}

void normalmatrix(float *result, float *src) {
	float tmp[16];
	float tmp2[16];
	int i, j;
	
	inverse(tmp, src);
	transpose(tmp2, tmp);
	for (j=0; j<3; j++) for (i=0; i<3; i++) {
		result[j*3+i] = tmp2[j*4+i];
	}
}

static float rocketcamerapos[3]={0.0, 0.0, 0.0};
static float rocketcameratgt[3]={0.0, 0.0, 0.0};

void rocketcamera(float *pmat, float *lmat, float znear, float zfar, double row) {
//	float pos[3], tgt[3], offs[3];
	float offs[3];
	float yvec[] = {0.0, 1.0, 0.0};
	float dist, rot, fov, roll;
	
	dist = SYNC(sync_camdist, row);
	rot = SYNC(sync_camrot, row);
	fov = SYNC(sync_camfov, row);
	roll = SYNC(sync_camroll, row);
	SYNC3(rocketcameratgt, sync_camxtgt, sync_camytgt, sync_camztgt, row);
	SYNC3(offs, sync_camxoffs, sync_camyoffs, sync_camzoffs, row);

	// projection
	identity(pmat);
	perspective(pmat, fov, znear, zfar);
	rotatef(pmat, roll/57.2957795, 0.0, 0.0, 1.0);
	
	// lookat
	identity(lmat);	
	rocketcamerapos[0] = offs[0]+sin(rot*pi)*dist;
	rocketcamerapos[1] = offs[1];
	rocketcamerapos[2] = offs[2]+cos(rot*pi)*dist;
	lookat(lmat, rocketcamerapos, rocketcameratgt, yvec);
}

void project2d(float *out, float *in, float *pmat, float *mvmat) {
	float mat[16];
	int i, j;

	for (i=0; i<4; i++) for (j=0; j<4; j++)
		mat[i*4+j]=
			pmat[0*4+j]*mvmat[i*4+0]+
			pmat[1*4+j]*mvmat[i*4+1]+
			pmat[2*4+j]*mvmat[i*4+2]+
			pmat[3*4+j]*mvmat[i*4+3];

	out[0]=in[0]*mat[0]+in[1]*mat[4]+in[2]*mat[8]+mat[12];
	out[1]=in[0]*mat[1]+in[1]*mat[5]+in[2]*mat[9]+mat[13];
	out[2]=in[0]*mat[3]+in[1]*mat[7]+in[2]*mat[11]+mat[15];
	out[0]/=out[2];
	out[1]/=out[2];
}

/////////////////////////////////////////////////////////////////////
// MESH STUFF

typedef struct {
  int vbo;
  int ibo;
  int vc, fc;
  BOOL tc;
  float *vertexdata;
  BOOL tangents;
} Mesh;


void calc_fnorm(float *v1, float *v2, float *v3, float *n) {
  n[0]=(v1[1]-v3[1])*(v2[2]-v3[2])-(v1[2]-v3[2])*(v2[1]-v3[1]);
  n[1]=(v1[2]-v3[2])*(v2[0]-v3[0])-(v1[0]-v3[0])*(v2[2]-v3[2]);
  n[2]=(v1[0]-v3[0])*(v2[1]-v3[1])-(v1[1]-v3[1])*(v2[0]-v3[0]);
  normalize(n);
}


float *calc_normals(unsigned short *indices, float *vertices, int fc, int vc) {
	float *v;
	unsigned short *i;
	int x;
	float *n=malloc(sizeof(float)*vc*3);

	v=vertices;
//	if (obj->orig_indices) i=obj->orig_indices;
//	else i=obj->indices;
	i=indices;

	for (x=0; x<vc*3; x++) n[x]=0.0;

	for (x=0; x<fc; x++) {
		unsigned short i1, i2, i3;
		float fnorm[3];

		i1=i[x*3+0];
		i2=i[x*3+1];
		i3=i[x*3+2];

		calc_fnorm(&v[i1*3], &v[i2*3], &v[i3*3], fnorm);

		n[i1*3+0]+=fnorm[0]; n[i1*3+1]+=fnorm[1]; n[i1*3+2]+=fnorm[2];
		n[i2*3+0]+=fnorm[0]; n[i2*3+1]+=fnorm[1]; n[i2*3+2]+=fnorm[2];
		n[i3*3+0]+=fnorm[0]; n[i3*3+1]+=fnorm[1]; n[i3*3+2]+=fnorm[2];
	}
/*
  if (obj->orig_indices) {
    for (x=0; x<obj->linkcnt; x++) {
      int l1, l2;

      l1=obj->vertex_links[x*2+0]; // splitattu verteksi
      l2=obj->vertex_links[x*2+1]; // originaali
      obj->normals[l1]=obj->normals[l2];
    }
  }
*/
	for (x=0; x<vc; x++) normalize(&n[x*3]);

	return n;
}

/*
void calc_ftangent(float *p1, float *p2, float *p3, float *uv1, float *uv2, float *uv3, float *tangent, float *bitangent) {
	float edge1[3], edge2[3];
	float edge1uv[2], edge2uv[2];

	edge1[0] = p2[0] - p1[0];
	edge1[1] = p2[1] - p1[1];
	edge1[2] = p2[2] - p1[2];

	edge2[0] = p3[0] - p1[0];
	edge2[1] = p3[1] - p1[1];
	edge2[2] = p3[2] - p1[2];

	edge1uv[0] = uv2[0] - uv1[0];
	edge1uv[1] = uv2[1] - uv1[1];
	edge2uv[0] = uv3[0] - uv1[0];
	edge2uv[1] = uv3[1] - uv1[1];

	float cp = edge1uv[1] * edge2uv[0] - edge1uv[0] * edge2uv[1];
	if (cp != 0.0) {
		float mul = 1.0 / cp;

		tangent[0] = (edge1[0] * -edge2uv[1] + edge2[0] * edge1uv[1]) * mul;
		tangent[1] = (edge1[1] * -edge2uv[1] + edge2[1] * edge1uv[1]) * mul;
		tangent[2] = (edge1[2] * -edge2uv[1] + edge2[2] * edge1uv[1]) * mul;

		bitangent[0] = (edge1[0] * -edge2uv[0] + edge2[0] * edge1uv[0]) * mul;
		bitangent[1] = (edge1[1] * -edge2uv[0] + edge2[1] * edge1uv[0]) * mul;
		bitangent[2] = (edge1[2] * -edge2uv[0] + edge2[2] * edge1uv[0]) * mul;
		normalize(tangent);
		normalize(bitangent);
	}
}

float calc_tangents(unsigned short *indides, float *texcoords, int fc, int vc)
{
	float *t=malloc(sizeof(float)*vc*6);
	int x;

	for (x=0; x<vc*6; x++) t[x]=0.0;

	for (x=0; x<fc; x++) {
		unsigned short i1, i2, i3;
		float ftan[3], fbitan[3];

		i1 = indices[x*3+0];
		i2 = indices[x*3+1];
		i3 = indices[x*3+2];

		calc_tangent(&vertices[i1*3], &vertices[i2*3], &vertices[i3*3], 
			&texcoords[i1*3], &texcoords[i2*3], &texcoords[i3*3], ftan, fbitan);

		t[i1*6+0] += ftan[0]; t[i1*6+1] += ftan[1]; t[i1*6+2] += ftan[2];
		t[i1*6+3] += fbitan[0]; t[i1*6+4] += fbitan[1]; t[i1*6+5] += fbitan[2];
		t[i2*6+0] += ftan[0]; t[i2*6+1] += ftan[1]; t[i2*6+2] += ftan[2];
		t[i2*6+3] += fbitan[0]; t[i2*6+4] += fbitan[1]; t[i2*6+5] += fbitan[2];
		t[i3*6+0] += ftan[0]; t[i3*6+1] += ftan[1]; t[i3*6+2] += ftan[2];
		t[i3*6+3] += fbitan[0]; t[i3*6+4] += fbitan[1]; t[i3*6+5] += fbitan[2];
	}

	for (x=0; x<vc; x++) {
		normalize(&n[x*6]);
		normalize(&n[x*6+3]);
	}

	return t;
}
*/

Mesh *loadmesh(unsigned short *indices, float *vertices, float *texcoords, float *srcnormals, int fc, int vc/*, BOOL tangents*/) {
	float *normals;
	Mesh *m = malloc(sizeof(Mesh));
	int i;
	int stride;
	
	if (texcoords) {
		stride = 8;
		m->tc = TRUE;
	} else {
		stride = 6;
		m->tc = FALSE;
	}

	if (srcnormals) {
		LOGI("Using source normals");
		normals = srcnormals;
	} else {
		LOGI("Calculating normals");
		normals = calc_normals(indices, vertices, fc, vc);
	}

	/*
	if (tangents)
	{
		if (!texcoords) LOGE("no uvs for tangent calculation!");
		else
		{
			calc_tangents(indices, vertices, fc, vc);
			stride += 6;
			m->tangents = TRUE;
		}
	}
	*/
	LOGI("loadmesh stride = %i", stride);

	// generate VBOs
	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);	

/* mapbuffer code...

	glBufferData(GL_ARRAY_BUFFER, vc*stride*sizeof(float), 0, GL_STATIC_DRAW);
	float *vbo_buffer = (GLvoid *)glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
	
	for (i=0; i<vc; i++) {
		vbo_buffer[i*stride+0] = vertices[i*3+0];		
		vbo_buffer[i*stride+1] = vertices[i*3+1];	
		vbo_buffer[i*stride+2] = vertices[i*3+2];
		vbo_buffer[i*stride+3] = normals[i*3+0];		
		vbo_buffer[i*stride+4] = normals[i*3+1];		
		vbo_buffer[i*stride+5] = normals[i*3+2];
		if (texcoords) {
			vbo_buffer[i*stride+6] = texcoords[i*2+0];
			vbo_buffer[i*stride+7] = texcoords[i*2+1];
		}
	}
	
	glUnmapBufferOES(GL_ARRAY_BUFFER); 
*/
	// Vertex data, regular way
	float *tmpbuffer = malloc(vc*stride*sizeof(float));
	for (i=0; i<vc; i++) {
		tmpbuffer[i*stride+0] = vertices[i*3+0];		
		tmpbuffer[i*stride+1] = vertices[i*3+1];	
		tmpbuffer[i*stride+2] = vertices[i*3+2];
		tmpbuffer[i*stride+3] = normals[i*3+0];		
		tmpbuffer[i*stride+4] = normals[i*3+1];		
		tmpbuffer[i*stride+5] = normals[i*3+2];
		if (texcoords) {
			tmpbuffer[i*stride+6] = texcoords[i*2+0];
			tmpbuffer[i*stride+7] = texcoords[i*2+1];
		}
	}

	glBufferData(GL_ARRAY_BUFFER, vc*stride*sizeof(float), tmpbuffer, GL_STATIC_DRAW);
	free(tmpbuffer);
	
	// create index buffer
	glGenBuffers(1, &m->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*fc*3, indices, GL_STATIC_DRAW);

	m->vc = vc;
	m->fc = fc;

	//free(normals);
	//free(texcoords);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	checkerror("loadmesh");
	return m;
}


void drawmesh(Mesh *m) {
	int poshandle, texhandle, normhandle;
	int stride;
	
	if (m->tc) stride = 8;
	else stride = 6;

	// VBOs
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);

	// shader binds
	// positions
	poshandle = glGetAttribLocation(currentshader, POSITION_ARRAY_NAME);
	glVertexAttribPointer(poshandle, 3, GL_FLOAT, GL_FALSE, sizeof(float)*stride, (GLvoid*)NULL);
    glEnableVertexAttribArray(poshandle);
	// normals
	normhandle = glGetAttribLocation(currentshader, NORMAL_ARRAY_NAME);
	if (normhandle!=-1) { // might not be used in shader
		glVertexAttribPointer(normhandle, 3, GL_FLOAT, GL_FALSE, sizeof(float)*stride, (GLvoid*)(sizeof(float)*3));
    	glEnableVertexAttribArray(normhandle);	
    }
	// texcoords
	if (m->tc) {
		texhandle = glGetAttribLocation(currentshader, TEXCOORD_ARRAY_NAME);
		if (texhandle!=-1) { // might not be used in shader
			glVertexAttribPointer(texhandle, 2, GL_FLOAT, GL_FALSE, sizeof(float)*stride, (GLvoid*)(sizeof(float)*6));
		    glEnableVertexAttribArray(texhandle);
		}
	} 
	glDrawElements(GL_TRIANGLES, m->fc*3, GL_UNSIGNED_SHORT, (GLvoid*)NULL);
    glDisableVertexAttribArray(poshandle);
    if (normhandle!=-1) glDisableVertexAttribArray(normhandle);
    if (m->tc && texhandle!=-1) glDisableVertexAttribArray(texhandle);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	checkerror("drawmesh");
}

Mesh *cube(float r) {
	Mesh *m = malloc(sizeof(Mesh));
	float *v = malloc(sizeof(float)*3*24);
	float *t = malloc(sizeof(float)*3*24);
	unsigned short *i = malloc(sizeof(unsigned short)*12*3);

	// ylos y+
	v[0*3+0]=-r; v[0*3+1]=r; v[0*3+2]=-r;
	v[1*3+0]= r; v[1*3+1]=r; v[1*3+2]=-r;
	v[2*3+0]= r; v[2*3+1]=r; v[2*3+2]= r;
	v[3*3+0]=-r; v[3*3+1]=r; v[3*3+2]= r;

	t[0*2+0]=0; t[0*2+1]=0;
	t[1*2+0]=1; t[1*2+1]=0;
	t[2*2+0]=1; t[2*2+1]=1;
	t[3*2+0]=0; t[3*2+1]=1;

	i[0*3+0]=0; i[0*3+1]=2; i[0*3+2]=1;
	i[1*3+0]=0; i[1*3+1]=3; i[1*3+2]=2;

    // alas y-
	v[4*3+0]=-r; v[4*3+1]=-r; v[4*3+2]=-r;
	v[5*3+0]= r; v[5*3+1]=-r; v[5*3+2]=-r;
	v[6*3+0]= r; v[6*3+1]=-r; v[6*3+2]= r;
	v[7*3+0]=-r; v[7*3+1]=-r; v[7*3+2]= r;

	t[4*2+0]=0; t[4*2+1]=0;
	t[5*2+0]=1; t[5*2+1]=0;
	t[6*2+0]=1; t[6*2+1]=1;
	t[7*2+0]=0; t[7*2+1]=1;

	i[2*3+0]=4; i[2*3+1]=5; i[2*3+2]=6;
	i[3*3+0]=4; i[3*3+1]=6; i[3*3+2]=7;

	// eteen z-
	v[ 8*3+0]=-r; v[ 8*3+1]=-r; v[ 8*3+2]=-r;
	v[ 9*3+0]= r; v[ 9*3+1]=-r; v[ 9*3+2]=-r;
	v[10*3+0]= r; v[10*3+1]= r; v[10*3+2]=-r;
	v[11*3+0]=-r; v[11*3+1]= r; v[11*3+2]=-r;

	t[ 8*2+0]=0; t[ 8*2+1]=0;
	t[ 9*2+0]=1; t[ 9*2+1]=0;
	t[10*2+0]=1; t[10*2+1]=1;
	t[11*2+0]=0; t[11*2+1]=1;

	i[4*3+0]=8; i[4*3+1]=10; i[4*3+2]=9;
	i[5*3+0]=8; i[5*3+1]=11; i[5*3+2]=10;

	// eteen z+
	v[12*3+0]=-r; v[12*3+1]=-r; v[12*3+2]=r;
	v[13*3+0]= r; v[13*3+1]=-r; v[13*3+2]=r;
	v[14*3+0]= r; v[14*3+1]= r; v[14*3+2]=r;
	v[15*3+0]=-r; v[15*3+1]= r; v[15*3+2]=r;

	t[12*2+0]=0; t[12*2+1]=0;
	t[13*2+0]=1; t[13*2+1]=0;
	t[14*2+0]=1; t[14*2+1]=1;
	t[15*2+0]=0; t[15*2+1]=1;

	i[6*3+0]=12; i[6*3+1]=13; i[6*3+2]=14;
 	i[7*3+0]=12; i[7*3+1]=14; i[7*3+2]=15;

	// vasen x+
	v[16*3+0]=r; v[16*3+1]=-1; v[16*3+2]=-1;
	v[17*3+0]=r; v[17*3+1]=-1; v[17*3+2]= 1;
	v[18*3+0]=r; v[18*3+1]= 1; v[18*3+2]= 1;
	v[19*3+0]=r; v[19*3+1]= 1; v[19*3+2]=-1;

	t[16*2+0]=0; t[16*2+1]=0;
	t[17*2+0]=1; t[17*2+1]=0;
	t[18*2+0]=1; t[18*2+1]=1;
	t[19*2+0]=0; t[19*2+1]=1;

	i[8*3+0]=16; i[8*3+1]=18; i[8*3+2]=17;
	i[9*3+0]=16; i[9*3+1]=19; i[9*3+2]=18;

	// oikea x-
	v[20*3+0]=-r; v[20*3+1]=-1; v[20*3+2]=-1;
	v[21*3+0]=-r; v[21*3+1]=-1; v[21*3+2]= 1;
	v[22*3+0]=-r; v[22*3+1]= 1; v[22*3+2]= 1;
	v[23*3+0]=-r; v[23*3+1]= 1; v[23*3+2]=-1;

	t[20*2+0]=0; t[20*2+1]=0;
	t[21*2+0]=1; t[21*2+1]=0;
	t[22*2+0]=1; t[22*2+1]=1;
	t[23*2+0]=0; t[23*2+1]=1;

	i[10*3+0]=20; i[10*3+1]=21; i[10*3+2]=22;
	i[11*3+0]=20; i[11*3+1]=22; i[11*3+2]=23;

	m = loadmesh(i, v, t, 0, 12, 24);
	return m;
}


Mesh *sphere(int xslices, int yslices, float rad) {
	int vc=(yslices+1)*(xslices+1);
	int fc=yslices*xslices*2;
	float *v=malloc(sizeof(float)*3*vc);
	float *t=malloc(sizeof(float)*2*vc);
	unsigned short *i=malloc(sizeof(unsigned short)*fc*3);
	Mesh *m = malloc(sizeof(Mesh));
	int x, y;

	for (y=0; y<yslices+1; y++) for (x=0; x<xslices+1; x++) {
		float a1=2.0*x*pi/(float)(xslices)*1.0;
		float a2=1.0*y*pi/(float)(yslices);

		v[(y*(xslices+1)+x)*3+0] = sin(a1)*sin(a2)*rad;
		v[(y*(xslices+1)+x)*3+1] = cos(a2)*rad;
		v[(y*(xslices+1)+x)*3+2] = cos(a1)*sin(a2)*rad;
		t[(y*(xslices+1)+x)*2+0] = (float)x/(float)xslices;
		t[(y*(xslices+1)+x)*2+1] = (float)y/(float)yslices;
	}

	for (y=0; y<yslices; y++) for (x=0; x<xslices; x++) {
		i[(y*xslices+x)*6+0]=(y+0)*(xslices+1)+x;
		i[(y*xslices+x)*6+1]=(y+1)*(xslices+1)+x;
		i[(y*xslices+x)*6+2]=(y+1)*(xslices+1)+x+1;
		i[(y*xslices+x)*6+3]=(y+0)*(xslices+1)+x;
		i[(y*xslices+x)*6+4]=(y+1)*(xslices+1)+x+1;
		i[(y*xslices+x)*6+5]=(y+0)*(xslices+1)+x+1;
	}

	m = loadmesh(i, v, t, 0, fc, vc);
	return m;
}


/////////////////////////////////////////////////////////////////////
// SHADERS


void useshader(int shader) {
	currentshader = shader;
	glUseProgram(shader);
}

void uniformi(char *name, int value) {
    glUniform1i(glGetUniformLocation(currentshader, name), value);
}

void uniform1f(char *name, float value) {
    glUniform1f(glGetUniformLocation(currentshader, name), value);
}

void uniform2fv(char *name, float *value) {
    glUniform2fv(glGetUniformLocation(currentshader, name), 1, value);
}

void uniform3fv(char *name, float *value) {
    glUniform3fv(glGetUniformLocation(currentshader, name), 1, value);
}

void uniform4fv(char *name, float *value) {
    glUniform4fv(glGetUniformLocation(currentshader, name), 1, value);
}

void uniformmatrix3fv(char *name, float *value) {
    glUniformMatrix3fv(glGetUniformLocation(currentshader, name), 1, GL_FALSE, value);
}

void uniformmatrix4fv(char *name, float *value) {
    glUniformMatrix4fv(glGetUniformLocation(currentshader, name), 1, GL_FALSE, value);
}

void bindtexture(char *name, int tnum, int tunit) {
	glActiveTexture(GL_TEXTURE0+tunit);
	glBindTexture(GL_TEXTURE_2D, tnum);
	glUniform1i(glGetUniformLocation(currentshader, name), tunit);
	checkerror("bindtexture");
}

void bindcubemap(char *name, int tnum, int tunit) {
	glActiveTexture(GL_TEXTURE0+tunit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tnum);
	glUniform1i(glGetUniformLocation(currentshader, name), tunit);
	checkerror("bindcubemap");
}
/*
void bindtexture3d(char *name, int tnum, int tunit) {
	glActiveTexture(GL_TEXTURE0+tunit);
	glBindTexture(GL_TEXTURE_3D, tnum);
	glUniform1i(glGetUniformLocation(currentshader, name), tunit);
	checkerror("bindtexture3d");
}
*/

GLuint compileshader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}


#ifndef FINAL // load from assets instead of sdcard

int filelength(FILE *f) {
  fseek(f, 0, SEEK_END);
  int sz = ftell(f);
  fseek(f, 0, SEEK_SET);
  return sz;
}

char *loadshadersrc(char *filename) {
	char *buf;
	FILE *f;
	int size;
	char fname[100];
	
	sprintf(fname, "/sdcard/tmp/%s", filename);
	
	f = fopen(fname, "rb");
	size = filelength(f);
	buf = malloc(size+1);
    fread(buf, size, 1, f);
	fclose(f);
	buf[size]=0;

	return buf;
}

#else

char *loadshadersrc(char *filename) {
	Assetloader a;
	char *src;
	char fname[100];

	sprintf(fname, "shaders/%s", filename);

	a = loadasset(fname);
	src = malloc(a.size+1);
	memcpy(src, a.data, a.size);
	src[a.size] = 0;
	free(a.data);
	return src;
}
#endif


GLuint loadshader(char *v_filename, char *f_filename) {
	char *pVertexSource, *pFragmentSource;

	pVertexSource = loadshadersrc(v_filename);
	pFragmentSource = loadshadersrc(f_filename);
 
    GLuint vertexShader = compileshader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
    	LOGI("VS compile error: %s", v_filename);
        return 0;
    }

    GLuint pixelShader = compileshader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
    	LOGI("FS compile error: %s", f_filename);
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkerror("glAttachShader");
        glAttachShader(program, pixelShader);
        checkerror("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    
    LOGI("shader (%i) %s/%s loaded", program, v_filename, f_filename);
    
    return program;
}


/////////////////////////////////////////////////////////////////////
// STATE MACHINE CRAP

#define BLEND 1
#define NOBLEND 2
#define CULL 4
#define NOCULL 8
#define DEPTH 16
#define NODEPTH 32
#define ALPHABLEND 64
#define MULTBLEND 128
#define MINUSBLEND 256
//#define READDEPTH 512

void renderflags(int flags) {

	if (flags&NOBLEND) {
		glDisable(GL_BLEND);
	} else if (flags&BLEND) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);	
	} else if (flags&ALPHABLEND) {
		glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else if (flags&MULTBLEND) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	} else if (flags&MINUSBLEND) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	}
		
	if (flags&CULL) {
		glEnable(GL_CULL_FACE);
	} else if (flags&NOCULL) {
		glDisable(GL_CULL_FACE);
	}
	
	if (flags&DEPTH) {
		 //glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	} else if (flags&NODEPTH) {
		//glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
	}
	/*
	} else if (flags&READDEPTH) {
		//glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
	}*/
}


#define CLAMP 1
#define WRAP 2
#define LINEAR 4
#define NEAREST 8
#define MIPMAP 16

void texflags(int flags) {

	if (flags&CLAMP) {
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	} else if (flags&WRAP) {
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);		
	}

	if (flags&LINEAR) {
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else if (flags&NEAREST) {
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	} else if (flags&MIPMAP) {
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

}

/////////////////////////////////////////////////////////////////////
// FBO

typedef struct {
	int fbonum;
	int texnum, dtexnum;
	int xres, yres;
} Fbo;


Fbo *framebuffer(int xres, int yres) {
	Fbo *f = malloc(sizeof(Fbo));
  
  	LOGI("new framebuffer %ix%i", xres, yres);
  
	f->xres = xres;
	f->yres = yres;  
    glGenFramebuffers(1, &f->fbonum);

	// color    
    glGenTextures(1, &f->texnum);
    glBindTexture(GL_TEXTURE_2D, f->texnum);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, f->xres, f->yres, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, f->xres, f->yres, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    texflags(CLAMP|NEAREST);
	checkerror("color attachment");
    glBindTexture(GL_TEXTURE_2D, 0);        	
	
	// depth
/*
    glGenTextures(1, &f->dtexnum);
	checkerror("a");
    glBindTexture(GL_TEXTURE_2D, f->dtexnum);
	checkerror("b");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, f->xres, f->yres, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	checkerror("c");
    texflags(CLAMP|LINEAR);    
	checkerror("depth attachment");
    glBindTexture(GL_TEXTURE_2D, 0);        
*/
	// depth
    glGenRenderbuffers(1, &f->dtexnum);
    glBindRenderbuffer(GL_RENDERBUFFER, f->dtexnum);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, f->xres, f->yres);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, f->fbonum);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, f->texnum, 0);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, f->dtexnum, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, f->dtexnum);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
    	LOGE("Framebuffer not complete");
    	if (status==GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) LOGE("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		if (status==GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) LOGE("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
		if (status==GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) LOGE("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		if (status==GL_FRAMEBUFFER_UNSUPPORTED) LOGE("GL_FRAMEBUFFER_UNSUPPORTED");
    }
    
    checkerror("framebuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return f;
}

void bindfbo(Fbo *f) {
	if (f) {
	    glBindFramebuffer(GL_FRAMEBUFFER, f->fbonum);	
		analaspect = (float)f->xres/(float)f->yres;
		glViewport(0, 0, f->xres, f->yres);
	} else {
	    glBindFramebuffer(GL_FRAMEBUFFER, 0);
		analaspect = (float)anal_xres/(float)anal_yres;
		glViewport(0, 0, anal_xres, anal_yres);	
	}
}

/////////////////////////////////////////////////////////////////////
// PARTICLES


#define FLTCONV(x) ((x) ^ (0U-((x)>>31)>>1) ^ 0x80000000)
#define DIGIT(v, p) ((FLTCONV(v[0])>>(p)*8)&255)

typedef struct {
  float f;
  unsigned int i;
} Sortthing;

static Sortthing radix_buffer1[262144];
static Sortthing radix_buffer2[262144];

void radixsort(unsigned int count) {
  unsigned *u;
  static unsigned int offsets[1024];
  unsigned int i, sum, t;

  if (count>262144) LOGE("too many stuffs to sort");
  assert(sizeof(float) == sizeof(unsigned));

  for (i = 0; i<1024; i++) offsets[i] = 0;

  for (i = 0; i<count; i++) {
    u = (unsigned *)&radix_buffer1[i].f;
    offsets[DIGIT(u, 0)    ]++;
    offsets[DIGIT(u, 1)+256]++;
    offsets[DIGIT(u, 2)+512]++;
    offsets[DIGIT(u, 3)+768]++;
  }

  sum = 0;
  for (i=0; i<1024; i++) {
    t = offsets[i];
    offsets[i] = sum;
    sum = (i&255) == 255 ? 0 : sum + t;
  }

  for (i=0; i<count; i++) u = (unsigned *)&radix_buffer1[i].f, radix_buffer2[offsets[DIGIT(u, 0)    ]++] = radix_buffer1[i];
  for (i=0; i<count; i++) u = (unsigned *)&radix_buffer2[i].f, radix_buffer1[offsets[DIGIT(u, 1)+256]++] = radix_buffer2[i];
  for (i=0; i<count; i++) u = (unsigned *)&radix_buffer1[i].f, radix_buffer2[offsets[DIGIT(u, 2)+512]++] = radix_buffer1[i];
  for (i=0; i<count; i++) u = (unsigned *)&radix_buffer2[i].f, radix_buffer1[offsets[DIGIT(u, 3)+768]++] = radix_buffer2[i];
}

#define MAXPARTICLES 16384


Mesh *loadparticles(float *src, unsigned short count, float rotate) {
	int x;
	float *v = malloc(sizeof(float)*count*32);
	unsigned short *i = malloc(sizeof(unsigned short)*count*6);
	Mesh *m = malloc(sizeof(Mesh));
	int stride = 8;

	if (count>MAXPARTICLES) {
		LOGE("TOO MANY PARTICLES DUMBASS");
		return 0;
	}
		
	for (x=0; x<count; x++) {
		float rot = x*rotate+pi*0.25;
		float *vv = &v[x*32];
		
		vv[0] = src[x*3+0]; vv[1] = src[x*3+1]; vv[2] = src[x*3+2];
		vv[3] = sin(rot); vv[4] = cos(rot); vv[5] = x;
		vv[6] = 0.0; vv[7] = 0.0;
		
		vv[8] = src[x*3+0]; vv[9] = src[x*3+1]; vv[10] = src[x*3+2];
		vv[11] = sin(rot+pi*0.5); vv[12] = cos(rot+pi*0.5); vv[13] = x;
		vv[14] = 1.0; vv[15] = 0.0;

		vv[16] = src[x*3+0]; vv[17] = src[x*3+1]; vv[18] = src[x*3+2];
		vv[19] = sin(rot+pi*1.0); vv[20] = cos(rot+pi*1.0); vv[21] = x;
		vv[22] = 1.0; vv[23] = 1.0;

		vv[24] = src[x*3+0]; vv[25] = src[x*3+1]; vv[26] = src[x*3+2];
		vv[27] = sin(rot+pi*1.5); vv[28] = cos(rot+pi*1.5); vv[29] = x;
		vv[30] = 0.0; vv[31] = 1.0;
	}

	// generate VBOs
	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);	
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*count*32, v, GL_STATIC_DRAW);
/* mapbuffer code ...
	float *vbo_buffer = (GLvoid *)glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
	memcpy(vbo_buffer, v, sizeof(float)*count*32);
	glUnmapBufferOES(GL_ARRAY_BUFFER); 
*/
	// create index buffer
	for (x=0; x<count; x++) {
		i[x*6+0] = x*4+0;
		i[x*6+1] = x*4+1;
		i[x*6+2] = x*4+2;
		i[x*6+3] = x*4+0;
		i[x*6+4] = x*4+2;
		i[x*6+5] = x*4+3;
	}
	
	glGenBuffers(1, &m->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*count*6, i, GL_DYNAMIC_DRAW);

	m->vc = count;
	m->fc = 0;
	m->vertexdata = src; // for sorting

	free(i);
	free(v);	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	checkerror("loadparticles");
	LOGI("loaded %i particles", count);
	
	return m;
}

#define PART_SORT 1


void drawparticles(Mesh *m, float *campos, float *camtgt, int flags) {
	int poshandle, texhandle, normhandle;
	int stride = 8;
	int x;
	float lookat[] = {camtgt[0]-campos[0], camtgt[1]-campos[1], camtgt[2]-campos[2]};
	static unsigned short tmpbuffer[MAXPARTICLES*6];
	
	normalize(lookat);

	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);

	if (flags&PART_SORT) {

		for (x=0; x<m->vc; x++) {
			float vpos[3];
			vpos[0] = m->vertexdata[x*3+0]-campos[0];
			vpos[1] = m->vertexdata[x*3+1]-campos[1];
			vpos[2] = m->vertexdata[x*3+2]-campos[2];
			radix_buffer1[x].f = dot(lookat, vpos);
			radix_buffer1[x].i = x;
		}
		radixsort(m->vc);

		// update index buffer
	    for (x=0; x<m->vc; x++) {
    		int i = radix_buffer1[(m->vc-1)-x].i;
    		tmpbuffer[x*6+0] = i*4+0;
	    	tmpbuffer[x*6+1] = i*4+1;
    		tmpbuffer[x*6+2] = i*4+2;
    		tmpbuffer[x*6+3] = i*4+0;
	    	tmpbuffer[x*6+4] = i*4+2;
    		tmpbuffer[x*6+5] = i*4+3;
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*m->vc*6, tmpbuffer, GL_DYNAMIC_DRAW);
				
		/* mapbuffer code
		unsigned short *idata = (GLvoid *)glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
	    for (x=0; x<m->vc; x++) {
    		int i = radix_buffer1[(m->vc-1)-x].i;
    		idata[x*6+0] = i*4+0;
	    	idata[x*6+1] = i*4+1;
    		idata[x*6+2] = i*4+2;
    		idata[x*6+3] = i*4+0;
	    	idata[x*6+4] = i*4+2;
    		idata[x*6+5] = i*4+3;
		}
		glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER); 
		*/
	}

	// shader binds
	// positions
	poshandle = glGetAttribLocation(currentshader, POSITION_ARRAY_NAME);
	glVertexAttribPointer(poshandle, 3, GL_FLOAT, GL_FALSE, sizeof(float)*stride, (GLvoid*)NULL);
    glEnableVertexAttribArray(poshandle);

	// normals
	normhandle = glGetAttribLocation(currentshader, NORMAL_ARRAY_NAME);
	glVertexAttribPointer(normhandle, 3, GL_FLOAT, GL_FALSE, sizeof(float)*stride, (GLvoid*)(sizeof(float)*3));
   	glEnableVertexAttribArray(normhandle);	

	// texcoords
	texhandle = glGetAttribLocation(currentshader, TEXCOORD_ARRAY_NAME);
	glVertexAttribPointer(texhandle, 2, GL_FLOAT, GL_FALSE, sizeof(float)*stride, (GLvoid*)(sizeof(float)*6));
    glEnableVertexAttribArray(texhandle);

	glDrawElements(GL_TRIANGLES, m->vc*6, GL_UNSIGNED_SHORT, (GLvoid*)NULL);

    glDisableVertexAttribArray(poshandle);
    glDisableVertexAttribArray(normhandle);
    glDisableVertexAttribArray(texhandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	checkerror("drawparticles");
}


/////////////////////////////////////////////////////////////////////
// RANDOM STUFF

void poly2d() {
	static float vertices[] = { -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f };
	static float texcoords[] = { 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	int poshandle, texhandle;

	poshandle = glGetAttribLocation(currentshader, POSITION_ARRAY_NAME);
    glVertexAttribPointer(poshandle, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(poshandle);
	texhandle = glGetAttribLocation(currentshader, TEXCOORD_ARRAY_NAME);
	glVertexAttribPointer(texhandle, 2, GL_FLOAT, GL_FALSE, 0, texcoords);     
    glEnableVertexAttribArray(texhandle);
    glDrawArrays(GL_TRIANGLES, 0, 6);    
}

void poly2dflip() {
	static float vertices[] = { -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f };
	static float texcoords[] = { 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
	int poshandle, texhandle;

	poshandle = glGetAttribLocation(currentshader, POSITION_ARRAY_NAME);
    glVertexAttribPointer(poshandle, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(poshandle);
	texhandle = glGetAttribLocation(currentshader, TEXCOORD_ARRAY_NAME);
	glVertexAttribPointer(texhandle, 2, GL_FLOAT, GL_FALSE, 0, texcoords);     
    glEnableVertexAttribArray(texhandle);
    glDrawArrays(GL_TRIANGLES, 0, 6);    
}



