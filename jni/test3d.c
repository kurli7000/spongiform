/*
finaalibuild:
x siivoo assetit
x remote export
x poista debug-versio
x boottaa
x EXIT_TIME
x anal.h:
  - #define FINAL
  - #define SYNC_PLAYER
x Manifest:
  - writing permission pois
  - internet-permission pois
x kopsaa rocketin data /sdcard/tmp -> assets
x release build
- 

x screenshotit 160x90, 400x...
- videocap: 
	- tee /sdcard/diskwrite
	- anal.h:sta definet
	- 32bit framebufferi: DEMOView.java
	- ilman filemaskia: adb pull /sdcard/diskwrite . 
x google play, screenshotit
- youtube
x infofile
- zip

*/




/////////////////////////////////////////////////////////////////////
// define stuff that goes into ANAL

// bass
#define biisi_filename "biisi.mp3"
#define BPM 122.0
#define EXIT_TIME 2900

// rocket
#define ROCKET_VARIABLES \
	const struct sync_track *sync_fxnum;\
	const struct sync_track *sync_fxbr;\
	const struct sync_track *sync_focus;\
	const struct sync_track *sync_glowbr;\
	const struct sync_track *sync_glowdim;\
	const struct sync_track *sync_ppos_x;\
	const struct sync_track *sync_ppos_y;\
	const struct sync_track *sync_overlaynum;\
	const struct sync_track *sync_overlayblend;\
	const struct sync_track *sync_overlayparam;\
	const struct sync_track *sync_overlaybr;\
	const struct sync_track *sync_wobblefrq;\
	const struct sync_track *sync_wobblepow;\
	const struct sync_track *sync_wobbleoffs;\
	const struct sync_track *sync_disko;\
	const struct sync_track *sync_diskopos;\
	const struct sync_track *sync_bork;

#define ROCKET_TRACKS \
	sync_fxnum = sync_get_track(rocket, "fxnum");\
	sync_fxbr = sync_get_track(rocket, "fxbr");\
	sync_focus = sync_get_track(rocket, "focus");\
	sync_glowbr = sync_get_track(rocket, "glowbr");\
	sync_glowdim = sync_get_track(rocket, "glowdim");\
	sync_ppos_x = sync_get_track(rocket, "ppos_x");\
	sync_ppos_y = sync_get_track(rocket, "ppos_y");\
	sync_overlaynum = sync_get_track(rocket, "overlay_num");\
	sync_overlayblend = sync_get_track(rocket, "overlay_blend");\
	sync_overlayparam = sync_get_track(rocket, "overlay_param");\
	sync_overlaybr = sync_get_track(rocket, "overlay_br");\
	sync_wobblefrq = sync_get_track(rocket, "wobble_frq");\
	sync_wobblepow = sync_get_track(rocket, "wobble_pow");\
	sync_wobbleoffs = sync_get_track(rocket, "wobble_offs");\
	sync_disko = sync_get_track(rocket, "disko");\
	sync_diskopos = sync_get_track(rocket, "diskopos");\
	sync_bork = sync_get_track(rocket, "bork");\
	
#define ROCKET_HOST "192.168.11.2"

// shader vertexattribpointer names
#define POSITION_ARRAY_NAME "a_position"
#define NORMAL_ARRAY_NAME "a_normal"
#define TEXCOORD_ARRAY_NAME "a_texcoord"
//#define TANGENT_ARRAY_NAME "a_tangents"
//#define BITANGENT_ARRAY_NAME "a_bitangents"


/////////////////////////////////////////////////////////////////////
// Android Native App Library (ANAL2.0b)
#include "anal.c"

// kludges
char errormessage[100]="you are not supposed to see this :)";
static BOOL NEED_TO_INIT_FBOS = TRUE; // FBO reload flag

/////////////////////////////////////////////////////////////////////////////////////////
// STUFF

#include "mandel.inc"
Mesh *m_mandel;

#include "blob1.inc"
Mesh *m_blob1;

#include "blob2.inc"
Mesh *m_blob2;

#include "white2.inc"
Mesh *m_white;

#include "spikeball.inc"
Mesh *m_spikeball;

Mesh *m_sphere;
Mesh *m_cube;
Mesh *m_part_sphere;
Mesh *m_part_lonkero;
Mesh *m_part_dirt;
Mesh *m_part_megablast;

// textures
int t_cell1;
int t_background1;
/*
int t_overlay1;
int t_overlay2;
int t_overlay3;
*/
//int t_cell2;
//int t_spherical1;
int t_cell3;
int t_envi1;
int t_spherical2;
int t_spherical3;
int t_spherical3b;
int t_envi2;
int t_dirt;
int t_background2;
int t_envi3;
int t_blueflare;
int t_blueflare2;
int t_kewlers;
int t_white_ao;
int t_lonkeroback;
int t_leiskat[25];
int t_blob1_ao;
int t_blob2_ao;
int t_whitecellback;
int t_envi4;
int t_backnoise;

// shaders
int s_basic;
int s_particle_sphere;
int s_particle_dof;
int s_bump;
int s_bump2;
int s_overlay;
int s_bumpenvi;
int s_bumpenvi2;
int s_minif_dof;
int s_dof;
int s_envi;
int s_particle_basic;
int s_zoomblur;
int s_minif16;
int s_glowcombine;
int s_particle_move;
int s_particle_megablast;
int s_particle_sphere2;
int s_breakshit;
int s_disko;

// fbos
Fbo *f_full1;
Fbo *f_half1;
Fbo *f_half2;
Fbo *f_quarter1;
//Fbo *f_sixth1;
Fbo *f_eighth1;
Fbo *f_eighth2;
Fbo *f_16;

/////////////////////////////////////////////////////////////////////////////////////////
// INIT

void load_shaders() {
    s_basic = loadshader("basic.vs", "basic.fs");
    s_particle_sphere = loadshader("particle_sphere.vs", "particle_sphere.fs");
    s_particle_dof = loadshader("particle_dof.vs","particle_dof.fs");
    s_bump = loadshader("bump.vs", "bump.fs");
    s_bump2 = loadshader("bump2.vs", "bump2.fs");
    s_overlay = loadshader("overlay.vs", "overlay.fs");
    s_bumpenvi = loadshader("bumpenvi.vs", "bumpenvi.fs");
    s_bumpenvi2 = loadshader("bumpenvi2.vs", "bumpenvi2.fs");
    s_minif_dof = loadshader("minif_dof.vs", "minif_dof.fs");
    s_dof = loadshader("dof.vs", "dof.fs");
    s_envi = loadshader("envi.vs", "envi.fs");
    s_particle_basic = loadshader("particle_basic.vs", "particle_basic.fs");
	s_zoomblur = loadshader("zoomblur.vs", "zoomblur.fs");   
	s_minif16 = loadshader("minif16.vs", "minif16.fs");
	s_glowcombine = loadshader("glowcombine.vs", "glowcombine.fs");
    s_particle_move = loadshader("particle_move.vs", "particle_move.fs");
    s_particle_megablast = loadshader("particle_megablast.vs", "particle_megablast.fs");
    s_particle_sphere2 = loadshader("particle_sphere2.vs", "particle_sphere2.fs");
    s_breakshit = loadshader("breakshit.vs", "breakshit.fs");
    s_disko = loadshader("disko.vs", "disko.fs");
}

void load_textures() {
	t_cell1 = loadtexture("cell1.png", 0);
	t_background1 = loadtexture("glow-cyan.png", 0);
	/*t_overlay1 = loadtexture("overlay-scope-grid-01.png", 0);
	t_overlay2 = loadtexture("overlay-scope-lines2.png", 0);
	t_overlay3 = loadtexture("overlay-scope-moire.png", 0);*/
	//t_cell2 = loadtexture("cell2_c.png", "cell2_a.png");
	//t_spherical1 = loadtexture("spherical1.png", 0);
	t_cell3 = loadtexture("cell3_c.png", "cell3_a.png");
	t_envi1 = loadtexture("envi1.png", 0);
	t_spherical2 = loadtexture("spherical2.png", 0);
	t_spherical3 = loadtexture("spherical3.png", 0);
	t_spherical3b = loadtexture("spherical3b.png", 0);		
	t_envi2 = loadtexture("envi2.png", 0);
	t_dirt = loadtexture("dirt.png", 0);
	t_background2 = loadtexture("back-orange1.png", 0);
	t_envi3 = loadtexture("envi3.png", 0);
	t_blueflare = loadtexture("blueflare.png", 0);
	t_blueflare2 = loadtexture("blueflare2.png", 0);
	t_kewlers = loadtexture("we-are-back-01.png", 0);
	t_white_ao = loadtexture("white2_ao.png", 0);
	t_lonkeroback = loadtexture("back-radial1.png", 0);
	t_blob1_ao = loadtexture("blob1_ao.png", 0);
	t_blob2_ao = loadtexture("blob2_ao.png", 0);
	t_whitecellback = loadtexture("back-radial2.png", 0);
	t_envi4 = loadtexture("envi4.png", 0);
	t_backnoise = loadtexture("back-noise.png", 0);

	t_leiskat[0] = loadtexture("logos-kewlers-01.png", 0);
	t_leiskat[1] = loadtexture("logos-square-01.png", 0);
	t_leiskat[2] = loadtexture("creds-elbee-01.png", 0);
	t_leiskat[3] = loadtexture("creds-kurli-01.png", 0);
	t_leiskat[4] = loadtexture("creds-tron-01.png", 0);
	t_leiskat[5] = loadtexture("greetz1-01.png", 0);
	t_leiskat[6] = loadtexture("greetz2-01.png", 0);
	t_leiskat[7] = loadtexture("greetz3-01.png", 0);
	t_leiskat[8] = loadtexture("overlay1-01.png", 0);
	t_leiskat[9] = loadtexture("overlay2-01.png", 0);
	t_leiskat[10] = loadtexture("overylay3-01.png", 0);
	t_leiskat[11] = loadtexture("overlay4-01.png", 0);
	t_leiskat[12] = loadtexture("rubbish1-01.png", 0);
	t_leiskat[13] = loadtexture("rubbish2-01.png", 0);
	t_leiskat[14] = loadtexture("rubbish3-01.png", 0);
	t_leiskat[15] = loadtexture("rubbish4-01.png", 0);
	t_leiskat[16] = loadtexture("overlay1.png", 0);
	t_leiskat[17] = loadtexture("overlay2.png", 0);
	t_leiskat[18] = loadtexture("overlay3.png", 0);
	t_leiskat[19] = loadtexture("overlay4.png", 0);
	t_leiskat[20] = loadtexture("overlay5.png", 0);
	t_leiskat[21] = loadtexture("overlay6.png", 0);
	t_leiskat[22] = loadtexture("overlay7.png", 0);
	t_leiskat[23] = loadtexture("overlay8.png", 0);
	t_leiskat[24] = loadtexture("greetz4-01.png", 0);
}

void load_meshes() {
	m_sphere = sphere(32, 32, 1);
	m_cube = cube(1.0);
	m_mandel = loadmesh(mandel_indices, mandel_vertices, 0, 0, mandel_fc, mandel_vc);
	m_blob1 = loadmesh(blob1_indices, blob1_vertices, blob1_texcoords, blob1_normals, blob1_fc, blob1_vc);
	m_blob2 = loadmesh(blob2_indices, blob2_vertices, blob2_texcoords, blob2_normals, blob2_fc, blob2_vc);
	m_white = loadmesh(white2_indices, white2_vertices, white2_texcoords, white2_normals, white2_fc, white2_vc);
	m_spikeball = loadmesh(spikeball_indices, spikeball_vertices, 0, spikeball_normals, spikeball_fc, spikeball_vc);
}

// load FBOs if surface was recreated
void load_fbos(int xres, int yres) {
	if (NEED_TO_INIT_FBOS) 
	{
		f_full1 = framebuffer(xres, yres);
		f_half1 = framebuffer(xres/2, yres/2);
		f_half2 = framebuffer(xres/2, yres/2);
		f_quarter1 = framebuffer(xres/4, yres/4);
		//f_sixth1 = framebuffer(xres/6, yres/8);
		f_eighth1 = framebuffer(xres/8, yres/8);
		f_eighth2 = framebuffer(xres/8, yres/8);
		f_16 = framebuffer(xres/16, yres/16);
	}	
	NEED_TO_INIT_FBOS = FALSE;
}


#define SPHERECNT 1024

void precalc_sphere_particles() {
	int x;
	float *pos = malloc(sizeof(float)*SPHERECNT*3);
	
	anal_seed = 2347821;
	for (x=0; x<SPHERECNT; x++) {
		float p[3];
		p[0] = ranf()-ranf();
		p[1] = ranf()-ranf();
		p[2] = ranf()-ranf();
		normalize(p);
		pos[x*3+0] = p[0];
		pos[x*3+1] = p[1];
		pos[x*3+2] = p[2];
	}
	//m_part_sphere = loadparticles(pos, SPHERECNT, 0.01715);
	m_part_sphere = loadparticles(pos, SPHERECNT, 0.0);
}


#define LONKEROCNT 1024

void precalc_lonkero() {
	float *v = malloc(sizeof(float)*LONKEROCNT*3);
	int x;
	
	anal_seed = 917291;
	
	static float pos[3], dir[3];
	dir[0] = ranf()-ranf();
	dir[1] = ranf()-ranf();
	dir[2] = ranf()-ranf();
	normalize(dir);

	pos[0] = ranf()-ranf();
	pos[1] = ranf()-ranf();
	pos[2] = ranf()-ranf();
	normalize(pos);
	pos[0] *= 16.0;
	pos[1] *= 16.0;
	pos[2] *= 16.0;

	for (x=0; x<LONKEROCNT; x++) {

		pos[0] += dir[0]*1.0;
		pos[1] += dir[1]*1.0;
		pos[2] += dir[2]*1.0;
		dir[0] += (ranf()-ranf())*0.666;
		dir[1] += (ranf()-ranf())*0.666;
		dir[2] += (ranf()-ranf())*0.666;

		if (x<LONKEROCNT/2) {
			if (vlen(pos) < 14.0) {
				dir[0] += pos[0]*0.05; 
				dir[1] += pos[1]*0.05;
				dir[2] += pos[2]*0.05;
			}

			if (vlen(pos) > 18.0) {
				dir[0] -= pos[0]*0.05;
				dir[1] -= pos[1]*0.05;
				dir[2] -= pos[2]*0.05;
			}
		} else {
			dir[0] -= pos[0]*0.001;
			dir[1] -= pos[1]*0.001;
			dir[2] -= pos[2]*0.001;
		}

		normalize(dir);

		v[x*3+0] = pos[0];
		v[x*3+1] = pos[1];
		v[x*3+2] = pos[2];
	}
				
	m_part_lonkero = loadparticles(v, LONKEROCNT, 1.9572);
}

#define DIRTCNT 256

void precalc_dirt_particles() {
	int x;
	float *pos = malloc(sizeof(float)*DIRTCNT*3);
	
	anal_seed = 712831;
	for (x=0; x<DIRTCNT; x++) {
		float p[3];
		p[0] = ranf()-ranf();
		p[1] = ranf()-ranf();
		p[2] = ranf()-ranf();
		normalize(p);
		float dist = sqrt(ranf()) * 64.0f;
		//float dist = ranf() * 64.0f;
		pos[x*3+0] = p[0] * dist;
		pos[x*3+1] = p[1] * dist;
		pos[x*3+2] = p[2] * dist;
	}
	//m_part_dirt = loadparticles(pos, DIRTCNT, 0.1715);
	m_part_dirt = loadparticles(pos, DIRTCNT, 0.0);
}


#define MEGABLASTCNT 4096

void precalc_megablast_particles() {
	int x, y;
	float *pos = malloc(sizeof(float)*MEGABLASTCNT*3);

	for (y=0; y<64; y++) for (x=0; x<64; x++) {
		float a1=2.0*x*pi/128.0;
		float a2=(1.0*y*pi+1.0*pi)/65.0;

		pos[(y*64+x)*3+0] = a1;//sin(a1)*sin(a2);
		pos[(y*64+x)*3+1] = a2;//cos(a2);
		pos[(y*64+x)*3+2] = 0.0;//cos(a1)*sin(a2);
	}
	m_part_megablast = loadparticles(pos, MEGABLASTCNT, 0);
}



// load all assets here, except FBOs
void init() {

	load_meshes();
 	precalc_sphere_particles();
 	precalc_lonkero();
 	precalc_dirt_particles();
 	precalc_megablast_particles();
	load_textures();
	load_shaders();
}


/////////////////////////////////////////////////////////////////////////////////////////
// FILTERS



void minification16(Fbo *src) {
	float psize[] = {1.0/src->xres, 1.0/src->yres};

	renderflags(NOCULL | NODEPTH | NOBLEND);
	useshader(s_minif16);
	bindtexture("t_color", src->texnum, 0); 
	texflags(LINEAR);
	uniform2fv("psize", psize);
	poly2d();
}

void minification_dof(Fbo *src) {
	float psize[] = {1.0/src->xres, 1.0/src->yres};

	renderflags(NOCULL | NODEPTH | NOBLEND);
	useshader(s_minif_dof);
	bindtexture("t_color", src->texnum, 0); 
	texflags(LINEAR);
	uniform2fv("psize", psize);
	poly2d();
}

void doffilter(Fbo *src1, Fbo *src2, Fbo *src3, Fbo *src4, float bork) {

	renderflags(NOCULL | NODEPTH | NOBLEND);

	bindfbo(src2);
	minification_dof(src1);
	bindfbo(src3);
	minification_dof(src2);
	bindfbo(src4);
	minification_dof(src3);
	bindfbo(0);

	useshader(s_dof);
	bindtexture("t_tex1", src1->texnum, 0);
	//texflags(LINEAR | CLAMP);
	texflags(NEAREST | CLAMP);
	bindtexture("t_tex2", src2->texnum, 1);
	//texflags(LINEAR | CLAMP);
	texflags(NEAREST | CLAMP);
	bindtexture("t_tex3", src3->texnum, 2);
	//texflags(LINEAR | CLAMP);
	texflags(NEAREST | CLAMP);
	bindtexture("t_tex4", src4->texnum, 3);
	//texflags(LINEAR | CLAMP);
	texflags(NEAREST | CLAMP);

	uniform1f("bork", bork);

	poly2d();
}

void bumpfilter(Fbo *src, int envi, double row) {
	float psize[] = {1.0/src->xres, 1.0/src->yres};
	float powah = SYNC(sync_wobblepow, row);

	renderflags(NOCULL | NODEPTH | NOBLEND);
	useshader(s_bump);
	bindtexture("t_color", src->texnum, 0); 
	texflags(LINEAR | CLAMP);
	bindtexture("t_envi", envi, 1);
	texflags(LINEAR);
	uniform2fv("texel", psize);
	uniform1f("time", row/64.0);
	uniform1f("powah", powah);
	uniform1f("br", SYNC(sync_fxbr, row));
	poly2d();
}

void bump2filter(Fbo *src, float br, float powah) {
	float psize[] = {1.0/src->xres, 1.0/src->yres};

	renderflags(NOCULL | NODEPTH | NOBLEND);
	useshader(s_bump2);
	bindtexture("t_color", src->texnum, 0); 
	texflags(LINEAR | CLAMP);
	uniform2fv("texel", psize);
	uniform1f("br", br);
	uniform1f("powah", powah);
	poly2d();
}

void overlay(int tnum, float br, int tflags, int flags) {

	renderflags(flags);
	useshader(s_overlay);
	bindtexture("t_color", tnum, 0);
	texflags(tflags);
	uniform1f("br", br); 
	poly2dflip();
}

void overlaydisko(int tnum, int tnum2, float br, int tflags, int flags, float disko, float diskopos) {
	float mat[16];

	identity(mat);

	renderflags(flags);
	useshader(s_disko);
	bindtexture("t_color", tnum, 0);
	texflags(tflags);
	bindtexture("t_color2", tnum2, 1);
	texflags(tflags);
	float brz[]={br, br, br, 1.0};
	uniform4fv("br", brz);	
	uniform1f("disko", disko);
	uniform1f("pos", diskopos);
	uniformmatrix4fv("mat", mat);
	poly2dflip();
}


void zoomblur(Fbo *f1, Fbo *f2, float zoomfactor, float *center) {
	float c2[2];

	renderflags(NOCULL | NODEPTH | NOBLEND);

	bindfbo(f2);
	useshader(s_zoomblur);
	bindtexture("t_color", f1->texnum, 0); 
	texflags(LINEAR);
	uniform1f("zoomfactor", zoomfactor);
	uniform2fv("center", center);
	poly2d();

	bindfbo(f1);
	renderflags(NOCULL | NODEPTH | NOBLEND);
	useshader(s_zoomblur);
	bindtexture("t_color", f2->texnum, 0); 
	texflags(LINEAR);
	uniform1f("zoomfactor", zoomfactor*4);
	uniform2fv("center", center);
	poly2d();

	bindfbo(0);
}

void glowcombine(int origtex, int blurtex, float fxbr, float glowbr, float glowdim) {
	useshader(s_glowcombine);
	bindtexture("t_orig", origtex, 0);
	texflags(NEAREST);
	bindtexture("t_blur", blurtex, 1);
	texflags(LINEAR);
	uniform1f("fxbr", fxbr);
	uniform1f("glowbr", glowbr);
	uniform1f("glowdim", glowdim);
	//uniform2fv("contrast", contrast);
	//uniform3fv("biits", biits);
	//bindtexture("t_leiska", t_overlay[SYNCi(sync_overlaynum, row)], 2);
	//texflags(LINEAR);
	//uniform1f("leiskabr", SYNC(sync_overlaybr, row));
	poly2d();
}

void breakshit(double row) {
	int num = SYNCi(sync_overlaynum, row);
	int blendmode = SYNCi(sync_overlayblend, row);
	int blend;
	float param = SYNC(sync_overlayparam, row);
	float br = SYNC(sync_overlaybr, row);

	if (num<0) return;

	if (blendmode == 0) blend = BLEND;
	else if (blendmode == 1) blend = MINUSBLEND;
	else if (blendmode == 2) blend = MULTBLEND;
	else blend = NOBLEND;

	renderflags(NOCULL|NODEPTH|blend);
	useshader(s_breakshit);
	bindtexture("t_color", t_leiskat[num], 0);
	texflags(NEAREST);
	uniform1f("param", param); 
	uniform1f("time", row/64.0);
	uniform1f("br", br);
	poly2dflip();
}


/////////////////////////////////////////////////////////////////////////////////////////
// EFFECTS


void drawdirt(float *mat, float *cmat, int blend, float brz) {
	float up[3], right[3];

	useshader(s_particle_basic);
	up[0] = cmat[1]; up[1] = cmat[5]; up[2] = cmat[9];
	right[0] = cmat[0]; right[1] = cmat[4];	right[2] = cmat[8];
	bindtexture("t_color", t_dirt, 0);
	uniformmatrix4fv("mat", mat);
	uniform3fv("up", up);
	uniform3fv("right", right);
	uniform1f("size", 1.0);
	float br[]={0.5 * brz, 0.5 * brz, 0.5 * brz, 0.0};
	uniform4fv("br", br);
	renderflags(NOCULL | blend | DEPTH);
	drawparticles(m_part_dirt, rocketcamerapos, rocketcameratgt, 0);
}



void mandelefu(double row) {
	float t = row/128.0;
	float mat[16], prmat[16], cmat[16], mvmat[16], nmat[9], omat[16];
//	float look[3];
	float br = SYNC(sync_fxbr, row);

	bindfbo(f_half1);
	
	clear(black);
	rocketcamera(prmat, cmat, 0.1, 2048.0, row);


	// tausta
	memcpy(mvmat, cmat, sizeof(float)*16);
	scalef(mvmat, 128.0, 128.0, 128.0);
	multmatrix(mat, prmat, mvmat);
	useshader(s_basic);
	bindtexture("t_color", t_spherical2, 0);
	uniformmatrix4fv("mat", mat);
	float brz[]={br, br, br, 1.0};
	uniform4fv("br", brz);
	renderflags(NOCULL | DEPTH | NOBLEND);
	drawmesh(m_sphere);	

	// obu
	identity(omat); // object matrix
	//scalef(omat, 32.0, 32.0, 32.0);
	//float rotdir[3]={1.0, 1.0, 1.0};
	//normalize(rotdir);
	//rotatef(omat, -t*2.0, rotdir[0], rotdir[1], rotdir[2]);	
	normalmatrix(nmat, omat); // normalmatrix
	multmatrix(mvmat, cmat, omat); // modelview matrix
	multmatrix(mat, prmat, mvmat); // modelviewprojection matrix
	//useshader(s_cubeenvi);
	useshader(s_bumpenvi);
	//bindcubemap("t_color", t_parkkis_256, 0);
	bindtexture("t_envi", t_envi1, 0);
	uniformmatrix4fv("mat", mat);
	uniformmatrix3fv("nmat", nmat);
	uniform1f("focus", SYNC(sync_focus, row));
	float wobble[3];
	wobble[0]=SYNC(sync_wobblefrq, row);
	wobble[1]=SYNC(sync_wobblepow, row);
	wobble[2]=sqrt(SYNC(sync_wobbleoffs, row));
	uniform3fv("wobble", wobble);
	uniform1f("br", br);
	//uniformmatrix4fv("omat", omat);
	//uniform3fv("campos", rocketcamerapos);
	renderflags(CULL | DEPTH | NOBLEND);
	drawmesh(m_mandel);

//	identity(omat);
//	multmatrix(mvmat, cmat, omat);
//	multmatrix(mat, prmat, mvmat);
	drawdirt(mat, cmat, MINUSBLEND, 1.0);

	doffilter(f_half1, f_quarter1, f_eighth1, f_16, SYNC(sync_bork, row));

}

void blobiefu(double row) {
	float t = row/128.0;
	float mat[16], prmat[16], cmat[16], mvmat[16], nmat[9], omat[16];
	int x;
	float br = SYNC(sync_fxbr, row);

	bindfbo(f_half1);
	
	clear(black);
	rocketcamera(prmat, cmat, 0.1, 512.0, row);

	memcpy(mvmat, cmat, sizeof(float)*16);
	renderflags(CULL | DEPTH | NOBLEND);
	useshader(s_envi);
	bindtexture("t_envi", t_envi2, 0);
	uniform1f("focus", SYNC(sync_focus, row));
	uniform1f("time", t);
	uniform1f("br", br);
	float wpow = SYNC(sync_wobblepow, row);
	float wobble[2];
	wobble[0] = SYNC(sync_wobblefrq, row);
	wobble[1] = wpow;
	uniform2fv("wobble", wobble);
	float col[]={wpow*0.6, wpow*0.8, wpow};
	uniform3fv("surface", col);

	// obut
	for (x=0; x<6; x++) {
		identity(omat); // object matrix
		translatef(omat, sin(x*2.41)*32.0, sin(x*5.172)*32.0, sin(x*3.572)*32.0);
		scalef(omat, 8.0, 8.0, 8.0);
		float rotdir[3]={sin(x*1.25), sin(x*2.53), sin(x*3.76)};
		normalize(rotdir);
		rotatef(omat, -t*2.0, rotdir[0], rotdir[1], rotdir[2]);	
		normalmatrix(nmat, omat); // normalmatrix
		multmatrix(mvmat, cmat, omat); // modelview matrix
		multmatrix(mat, prmat, mvmat); // modelviewprojection matrix 

		uniformmatrix4fv("mat", mat);
		uniformmatrix3fv("nmat", nmat);

		if (x&1) bindtexture("t_diffuse", t_blob1_ao, 1); 
		else bindtexture("t_diffuse", t_blob2_ao, 1);

		if (x&1) drawmesh(m_blob1);
		else drawmesh(m_blob2);
	}
	
	// tausta
	memcpy(mvmat, cmat, sizeof(float)*16);
	scalef(mvmat, 128.0, 128.0, 128.0);
	multmatrix(mat, prmat, mvmat);
	useshader(s_disko);
	bindtexture("t_color", t_spherical3, 0);
	bindtexture("t_color2", t_spherical3b, 1);
	uniformmatrix4fv("mat", mat);
	//uniform1f("br", br);
	float brz[]={br, br, br, 1.0};
	uniform4fv("br", brz);	
	uniform1f("disko", SYNC(sync_disko, row));
	uniform1f("pos", SYNC(sync_diskopos, row));
	renderflags(NOCULL | DEPTH | NOBLEND);
	drawmesh(m_sphere);	


	//memcpy(mvmat, cmat, sizeof(float)*16);
	multmatrix(mat, prmat, cmat);
	drawdirt(mat, cmat, BLEND, br);

	doffilter(f_half1, f_quarter1, f_eighth1, f_16, SYNC(sync_bork, row));

}



void lonkeroefu(double row) {
	float t = row/128.0;
	float mat[16], prmat[16], cmat[16], mvmat[16];
	float up[3], right[3];

	bindfbo(f_half1);
	
	clear(black);
	overlay(t_lonkeroback, 1.0, 0, NOCULL|NODEPTH|NOBLEND);

	rocketcamera(prmat, cmat, 0.1, 256.0, row);
/*
	// tausta
	memcpy(mvmat, cmat, sizeof(float)*16);
	scalef(mvmat, 128.0, 128.0, 128.0);
	multmatrix(mat, prmat, mvmat);
	useshader(s_basic);
	bindtexture("t_color", t_spherical1, 0);
	uniformmatrix4fv("mat", mat);
	uniform1f("br", 1.0);
	renderflags(NOCULL | DEPTH | NOBLEND);
	drawmesh(m_sphere);
*/

	// partikkelit
	multmatrix(mat, prmat, cmat); // modelviewprojection matrix
	useshader(s_particle_dof);
	up[0] = cmat[1]; up[1] = cmat[5]; up[2] = cmat[9];
	right[0] = cmat[0]; right[1] = cmat[4];	right[2] = cmat[8];
	bindtexture("t_color", t_cell3, 0);
	uniformmatrix4fv("mat", mat);
	uniform3fv("up", up);
	uniform3fv("right", right);
	uniform1f("size", 2.0);
	uniform1f("focus", SYNC(sync_focus, row));
	uniform1f("time", row/64.0);
	uniform1f("grow", SYNC(sync_bork, row));
	renderflags(NOCULL | ALPHABLEND | NODEPTH);
	drawparticles(m_part_lonkero, rocketcamerapos, rocketcameratgt, PART_SORT);

	bindfbo(0);
	bump2filter(f_half1, SYNC(sync_fxbr, row), 1.0);

//	overlay(t_overlay2, 1.0, 0, MULTBLEND);
}



void sphereparticles(double row) {
	float t = row/128.0;
	float mat[16], prmat[16], cmat[16], mvmat[16];
	float up[3], right[3];

	bindfbo(f_quarter1);

	clear(black);
	rocketcamera(prmat, cmat, 0.1, 512.0, row);

	// partikkelit
	multmatrix(mat, prmat, cmat); // modelviewprojection matrix
	useshader(s_particle_sphere);
	up[0] = cmat[1]; up[1] = cmat[5]; up[2] = cmat[9];
	right[0] = cmat[0]; right[1] = cmat[4];	right[2] = cmat[8];
	bindtexture("t_color", t_cell1, 0);
	uniformmatrix4fv("mat", mat);
	uniform3fv("up", up);
	uniform3fv("right", right);
	uniform1f("size", 1.0);
	uniform1f("time", t);
	renderflags(NOCULL | BLEND | NODEPTH);
	drawparticles(m_part_sphere, rocketcamerapos, rocketcameratgt, 0);

	bindfbo(0);
	bumpfilter(f_quarter1, t_background1, row);

	//overlay(t_overlay3, 1.0, 0, MULTBLEND);

}


void zoomparticles(double row) {
	float t = row/128.0;
	float mat[16], prmat[16], cmat[16], mvmat[16];
	float up[3], right[3];

	bindfbo(f_half1);
	clear(black);
	overlay(t_background2, 1.0, 0, NOCULL | NODEPTH | NOBLEND);

	rocketcamera(prmat, cmat, 0.1, 512.0, row);

	// partikkelit
	multmatrix(mat, prmat, cmat); // modelviewprojection matrix
	useshader(s_particle_move);
	up[0] = cmat[1]; up[1] = cmat[5]; up[2] = cmat[9];
	right[0] = cmat[0]; right[1] = cmat[4];	right[2] = cmat[8];
	bindtexture("t_color", t_cell1, 0);
	uniformmatrix4fv("mat", mat);
	uniform3fv("up", up);
	uniform3fv("right", right);
	uniform1f("size", 1.0);
	uniform1f("time", t);
	renderflags(NOCULL | MINUSBLEND | NODEPTH);
	drawparticles(m_part_sphere, rocketcamerapos, rocketcameratgt, 0);

	bindfbo(f_eighth1);
	minification16(f_half1);
	float center[] = {0.0, 0.0};
	center[0] = SYNC(sync_ppos_x, row);
	center[1] = SYNC(sync_ppos_y, row);
	zoomblur(f_eighth1, f_eighth2, 0.05, center);

	glowcombine(f_half1->texnum, f_eighth1->texnum, SYNC(sync_fxbr, row), SYNC(sync_glowbr, row), SYNC(sync_glowdim, row));

}

void whitecellefu(double row) {
	float t = row/128.0;
	float mat[16], prmat[16], cmat[16], mvmat[16], nmat[9], omat[16];
	float br = SYNC(sync_fxbr, row);

	bindfbo(f_half1);
	
	clear(black);
	//overlay(t_whitecellback, br, 0, NOCULL|NODEPTH|NOBLEND);
	overlaydisko(t_whitecellback, t_backnoise, br, 0, NOCULL|NODEPTH|NOBLEND, SYNC(sync_disko, row), SYNC(sync_diskopos, row));

	rocketcamera(prmat, cmat, 0.1, 2048.0, row);
/*
	// tausta
	memcpy(mvmat, cmat, sizeof(float)*16);
	scalef(mvmat, 128.0, 128.0, 128.0);
	multmatrix(mat, prmat, mvmat);
	useshader(s_basic);
	bindtexture("t_color", t_spherical1, 0);
	uniformmatrix4fv("mat", mat);
	uniform1f("br", 1.0);
	renderflags(NOCULL | DEPTH | NOBLEND);
	drawmesh(m_sphere);	
*/
	// obu
	identity(omat); // object matrix
	scalef(omat, 32.0, 32.0, 32.0);
	float rotdir[3]={1.0, -1.0, 1.0};
	normalize(rotdir);
	rotatef(omat, t*3.0, rotdir[0], rotdir[1], rotdir[2]);	
	normalmatrix(nmat, omat); // normalmatrix
	multmatrix(mvmat, cmat, omat); // modelview matrix
	multmatrix(mat, prmat, mvmat); // modelviewprojection matrix
	useshader(s_envi);
	bindtexture("t_envi", t_envi3, 0);
	bindtexture("t_diffuse", t_white_ao, 1);
	uniformmatrix4fv("mat", mat);
	uniformmatrix3fv("nmat", nmat);
	uniform1f("focus", SYNC(sync_focus, row));
	uniform1f("time", t);
	float wpow = SYNC(sync_wobblepow, row);
	float col[]={wpow*0.2, wpow*0.2, wpow};
	uniform3fv("surface", col);
	float wobble[2];
	wobble[0] = SYNC(sync_wobblefrq, row);
	wobble[1] = wpow;	
	uniform2fv("wobble", wobble);
	uniform1f("br", br);
	renderflags(CULL|DEPTH|NOBLEND);
	drawmesh(m_white);

	// dirt
	identity(omat); // object matrix
	scalef(omat, 2.0, 2.0, 2.0);
	multmatrix(mvmat, cmat, omat); // modelview matrix
	multmatrix(mat, prmat, mvmat); // modelviewprojection matrix
	drawdirt(mat, cmat, MINUSBLEND, 1.0);

	doffilter(f_half1, f_quarter1, f_eighth1, f_16, SYNC(sync_bork, row));

}


void megablast(double row) {
	float t = row/128.0;
	float mat[16], prmat[16], cmat[16], mvmat[16];
	float up[3], right[3];

	bindfbo(f_half1);
	clear(black);
	rocketcamera(prmat, cmat, 0.1, 256.0, row);

	// partikkelit
	multmatrix(mat, prmat, cmat); // modelviewprojection matrix
	useshader(s_particle_megablast);
	up[0] = cmat[1]; up[1] = cmat[5]; up[2] = cmat[9];
	right[0] = cmat[0]; right[1] = cmat[4];	right[2] = cmat[8];

	float center[] = {0.0, 0.0};
	center[0] = SYNC(sync_ppos_x, row);
	center[1] = SYNC(sync_ppos_y, row);

	bindtexture("t_color", t_blueflare, 0);
	uniformmatrix4fv("mat", mat);
	uniform3fv("up", up);
	uniform3fv("right", right);
	uniform1f("size", 0.75);
	uniform1f("time", t);
	float wobble[3];
	wobble[0]=SYNC(sync_wobblefrq, row);
	wobble[1]=SYNC(sync_wobblepow, row);
	wobble[2]=SYNC(sync_wobbleoffs, row);
	uniform3fv("wobble", wobble);
	uniform2fv("middle", center);
	renderflags(NOCULL | BLEND | NODEPTH);
	drawparticles(m_part_megablast, rocketcamerapos, rocketcameratgt, 0);

	bindfbo(f_eighth1);
	minification16(f_half1);
	center[0] = center[0]*0.5+0.5;
	center[1] = center[1]*0.5+0.5;
	zoomblur(f_eighth1, f_eighth2, 0.04, center);

	bindfbo(f_half2);
	glowcombine(f_half1->texnum, f_eighth1->texnum, SYNC(sync_fxbr, row), SYNC(sync_glowbr, row), SYNC(sync_glowdim, row));
	bindfbo(0);
	bump2filter(f_half2, 1.0f, SYNC(sync_bork, row));


}


void logofx(double row) {
	float t = row/128.0;
	float mat[16], prmat[16], cmat[16], mvmat[16];
	float up[3], right[3];

	bindfbo(f_half1);
	clear(black);
	rocketcamera(prmat, cmat, 0.1, 512.0, row);

	// partikkelit
	multmatrix(mat, prmat, cmat); // modelviewprojection matrix
	useshader(s_particle_sphere2);
	up[0] = cmat[1]; up[1] = cmat[5]; up[2] = cmat[9];
	right[0] = cmat[0]; right[1] = cmat[4];	right[2] = cmat[8];
	bindtexture("t_color", t_blueflare2, 0);
	uniformmatrix4fv("mat", mat);
	uniform3fv("up", up);
	uniform3fv("right", right);
	uniform1f("size", 1.0);
	uniform1f("time", t);
	float center[3];
	center[0]=SYNC(sync_ppos_x, row);
	center[1]=SYNC(sync_ppos_y, row);
	center[2]=0.0;
	uniform3fv("center", center);
	renderflags(NOCULL | BLEND | NODEPTH);
	drawparticles(m_part_sphere, rocketcamerapos, rocketcameratgt, 0);

	overlay(t_kewlers, 1.0, 0, MULTBLEND);

	bindfbo(f_eighth1);
	minification16(f_half1);
	float center2d[2];
	project2d(center2d, center, prmat, cmat);
	center2d[0]=center2d[0]*0.5+0.5;
	center2d[1]=center2d[1]*0.5+0.5;
	zoomblur(f_eighth1, f_eighth2, 0.033333, center2d);

	glowcombine(f_half1->texnum, f_eighth1->texnum, SYNC(sync_fxbr, row), SYNC(sync_glowbr, row), SYNC(sync_glowdim, row));

}


void spikeballefu(double row) {
	float t = row/128.0;
	float mat[16], prmat[16], cmat[16], mvmat[16], nmat[9], omat[16];
	float br = SYNC(sync_fxbr, row);

	bindfbo(f_half1);
	
	clear(black);
	rocketcamera(prmat, cmat, 0.1, 2048.0, row);


	// obu
	identity(omat); // object matrix
	scalef(omat, 32.0, 32.0, 32.0);
	float rotdir[3]={1.0, 1.0, 1.0};
	normalize(rotdir);
	rotatef(omat, t, rotdir[0], rotdir[1], rotdir[2]);	
	normalmatrix(nmat, omat); // normalmatrix
	multmatrix(mvmat, cmat, omat); // modelview matrix
	multmatrix(mat, prmat, mvmat); // modelviewprojection matrix
	//useshader(s_cubeenvi);
	useshader(s_bumpenvi2);
	//bindcubemap("t_color", t_parkkis_256, 0);
	bindtexture("t_envi", t_envi4, 0);
	uniformmatrix4fv("mat", mat);
	uniformmatrix3fv("nmat", nmat);
	uniform1f("focus", SYNC(sync_focus, row));
	float wobble[3];
	wobble[0]=SYNC(sync_wobblefrq, row);
	wobble[1]=SYNC(sync_wobblepow, row);
	wobble[2]=sqrt(SYNC(sync_wobbleoffs, row));
	uniform3fv("wobble", wobble);
	uniform1f("br", br);
	//uniformmatrix4fv("omat", omat);
	//uniform3fv("campos", rocketcamerapos);
	renderflags(CULL | DEPTH | NOBLEND);
	drawmesh(m_spikeball);

	// tausta
	memcpy(mvmat, cmat, sizeof(float)*16);
	scalef(mvmat, 128.0, 128.0, 128.0);
	multmatrix(mat, prmat, mvmat);
	useshader(s_disko);
	bindtexture("t_color", t_spherical3, 0);
	bindtexture("t_color2", t_spherical3b, 1);
	uniformmatrix4fv("mat", mat);
	//uniform1f("br", br);
	float brz[]={br, br, br, 0.0};
	uniform4fv("br", brz);
	uniform1f("disko", SYNC(sync_disko, row));	
	uniform1f("pos", SYNC(sync_diskopos, row));
	renderflags(NOCULL | DEPTH | NOBLEND);
	drawmesh(m_cube);	

	// dirt
	identity(omat); // object matrix
	scalef(omat, 2.0, 2.0, 2.0);
	multmatrix(mvmat, cmat, omat); // modelview matrix
	multmatrix(mat, prmat, mvmat); // modelviewprojection matrix
	drawdirt(mat, cmat, BLEND, br);

	doffilter(f_half1, f_quarter1, f_eighth1, f_16, SYNC(sync_bork, row));

}





/////////////////////////////////////////////////////////////////////////////////////////
// MAINLOOP


void step(double row) {
	int fxnum = SYNCi(sync_fxnum, row);

	if (fxnum == 0) sphereparticles(row);
	else if (fxnum == 1) lonkeroefu(row);
	else if (fxnum == 2) mandelefu(row);
	else if (fxnum == 3) blobiefu(row);
	else if (fxnum == 4) zoomparticles(row);
	else if (fxnum == 5) whitecellefu(row);
	else if (fxnum == 6) megablast(row);
	else if (fxnum == 7) logofx(row);
	else if (fxnum == 8) spikeballefu(row);

	breakshit(row);
}



/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// JAVA SCHEISSE


// TODO: siirrä anaaliin

jboolean Java_com_kewlers_spongiform_DEMOLib_initgl(JNIEnv * env, jobject obj, jobject assetManager) {
    int r, g, b, a, d, s;
    GLint err;
    
	if (stream) {
		LOGI("bass was open, closing now");
		close_bass();
	}

    LOGI("# START ######################################################");
    LOGI("init assmgr and bass");

    // get assetmanager
    assmgr = AAssetManager_fromJava(env, assetManager);
    if (!assmgr) {
    	sprintf(errormessage, "Assetmanager init failed");
    	return FALSE;
    }
    
   	if (!init_bass()) {
   		sprintf(errormessage, "BASS init failed");
   		return FALSE;
   	}

    LOGI("init graphics");
	initanal();    
/*	
	if (!checkext("GL_OES_mapbuffer")) {
		sprintf(errormessage, "Required extension GL_OES_mapbuffer not supported, sorry");
		return FALSE;
	}

	if (!checkext("GL_IMG_texture_npot")) {
		sprintf(errormessage, "Required extension GL_IMG_texture_npot not supported, sorry");
		return FALSE;
	}
*/		

    printglstring("Version", GL_VERSION);
   	printglstring("Vendor", GL_VENDOR);
    printglstring("Renderer", GL_RENDERER);
    printglstring("Extensions", GL_EXTENSIONS);
	glGetIntegerv(GL_RED_BITS, &r);
	glGetIntegerv(GL_GREEN_BITS, &g);
	glGetIntegerv(GL_BLUE_BITS, &b);
	glGetIntegerv(GL_ALPHA_BITS, &a);
	glGetIntegerv(GL_DEPTH_BITS, &d);
	glGetIntegerv(GL_STENCIL_BITS, &s);
	LOGI("GL_BITS (r/g/b/a/depth/stencil): %i/%i/%i/%i/%i/%i", r, g, b, a, d, s);

	// load stuff
	init();
	
	err = glGetError();
	if (err) {
		sprintf(errormessage, "glError 0x%x after init", e);
		return FALSE;	
	}
	
	LOGI("start playing");
	bass_startplaying();

	// flag for onSurfaceChanged event that FBOs need reloading
	NEED_TO_INIT_FBOS = TRUE;		

	return TRUE;
}



static float avgfps_fps=0.0;
static float avgfps_cnt=0.0;

jboolean Java_com_kewlers_spongiform_DEMOLib_step(JNIEnv * env, jobject obj) {
	static long prevmsec, msec;
	static int frames=0;

#ifdef DISKWRITE
	// change to 24bit surface from DEMOView.java before using this
	static int dw_frame = DISKWRITE_START;
	double row = dw_frame / DISKWRITE_FPS / 60.0 * BPM * rpb;
	//bass_set_row((void*)stream, row);
#else
	double row = bass_get_row(stream);
#endif

	ONCE(	
		prevmsec=getmsec();
	);
	BASS_Update(0);
	step(row);
	msec = getmsec();
	if (msec >= (prevmsec+1000)) {
		prevmsec = msec;
		LOGI("fps: %i", frames);
		avgfps_fps+=frames;
		avgfps_cnt+=1.0;
		frames = 0;
	} else frames++;
#ifndef SYNC_PLAYER
	if (sync_update(rocket, (int)floor(row))) sync_connect(rocket, ROCKET_HOST, SYNC_DEFAULT_PORT);
#endif

#ifdef FINAL
	if (row >= EXIT_TIME) return FALSE;
#endif

#ifdef DISKWRITE
	checkerror("effect");
	glFlush();
	checkerror("flush");
	char filename[128];
	sprintf(filename, "/sdcard/diskwrite/kwl_%05i.bmp", dw_frame);
	writeBmp(filename, anal_xres, anal_yres);
	dw_frame++;
	checkerror("writeBmp");
	if (dw_frame >= DISKWRITE_END) return FALSE;
#endif

	return TRUE;
}

void Java_com_kewlers_spongiform_DEMOLib_stop(JNIEnv * env, jobject obj) {
	close_bass();
	LOGI("avg fps = %5.2f", avgfps_fps/avgfps_cnt);
    LOGI("# END ########################################################");
}

void Java_com_kewlers_spongiform_DEMOLib_pause(JNIEnv * env, jobject obj, jboolean pause) {
	if (pause) BASS_ChannelPause((HSTREAM)stream);
	else BASS_ChannelPlay((HSTREAM)stream, FALSE);
}

void Java_com_kewlers_spongiform_DEMOLib_touchevent(JNIEnv * env, jobject obj) {
#ifndef FINAL
	load_shaders();
#endif
}

void Java_com_kewlers_spongiform_DEMOLib_resizegl(JNIEnv * env, jobject obj, jint width, jint height) {
	resizeanal(width, height);
	load_fbos(width, height);
}

jstring *Java_com_kewlers_spongiform_DEMOLib_getmessage(JNIEnv * env, jobject obj) {
	return (*env)->NewStringUTF(env, errormessage);
}

