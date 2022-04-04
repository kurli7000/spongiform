#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//#include <io.h>

typedef struct {
  float x, y, z;
  int k;
} vertex;

typedef struct {
  int v[3];
  int n[3];
  int used;
} face;

typedef struct {
  int v[2];
  int f[2];
  void *next;
  float sharpness;
} edge;

static char *tempbuf;
int datapos=0;

static vertex *temp_vlist;
static vertex *temp_vlist_x;
static face *temp_flist;
static face *temp_flist2;
static vertex *temp_tlist;
static vertex *temp_tlist_x;
//static vertex *temp_clist;
static edge *temp_elist;
static vertex *temp_nlist;
static vertex *temp_nlist_x;

static face *orig_flist;
int orig_fpos=0;
int vertices_splitted=0;
static int *muthas; 
int muthapos=0;

//static int mapped=0;
static int introkoko=0;
static int neighbours=0;
static int yzflip=0;
static int optimize=1;
//static int vertexcolors=0;
static int find_edges=0;
static int binary=0;
static int flat=0;

static int vertexpos_x=0;
static int texturepos_x=0;
static int normalpos_x=0;
static int vertexpos=0;
static int facepos=0;
static int facepos2=0;
static int edgepos=0;
int size=0;



#define MAX_VERTS 65535
#define MAX_FACES 200000
#define MAX_EDGES 200000

#define MAX_ATTACH 32

/********************************
********  vcache stuff  *********
********************************/


#define VCACHE 16

static int cache[VCACHE];
static int cachepos=0;
int bmisses=0, bhits=0;
int amisses=0, ahits=0;


int in_cache(int vertex) {
  int x;

  for (x=0; x<VCACHE; x++) {
    if (vertex==cache[x]) return 1; 
  }

  return 0;
}


int cache_hits(int face, int kumpi) {
  int hits=0;
  int x;

  for (x=0; x<3; x++) {
    if (kumpi==0) {
      hits+=in_cache(temp_flist[face].v[x]);
    } else {
      hits+=in_cache(temp_flist2[face].v[x]);
    }
  }

  return hits;
}


void draw_vertex(int vertex) {

  if (!in_cache(vertex)) {
    cache[cachepos]=vertex;
    cachepos=(cachepos+1)%VCACHE;
  }
}


void draw_face(int face, int kumpi) {
  int x;

  for (x=0; x<3; x++) {
    if (kumpi==0) {
      draw_vertex(temp_flist[face].v[x]);
    } else {
      draw_vertex(temp_flist2[face].v[x]);
    }
  }
}


void next_face() {
  int x, hits;

  for (hits=3; hits>=0; hits--) {
    for (x=0; x<facepos; x++) if (temp_flist[x].used==0) {
      if (cache_hits(x, 0)==hits) {
        temp_flist[x].used=1;
        temp_flist2[facepos2].v[0]=temp_flist[x].v[0];
        temp_flist2[facepos2].v[1]=temp_flist[x].v[1];
        temp_flist2[facepos2].v[2]=temp_flist[x].v[2];
        draw_face(facepos2, 1);
        facepos2++;
        return;
      }
    }
  }
}


void clear_cache() {
  int x;

  cachepos=0;

  for (x=0; x<VCACHE; x++) cache[x]=-1;
}





int handle_edge(edge *E, int pieni, int iso, int face) {

  // jos edgelistan eka slotti k‰ytt‰m‰tt‰, otetaan k‰yttˆˆn ja siirryt‰‰n seuraavaan edgeen
  if (E->v[0]==-1) {
    E->v[0]=pieni;
    E->v[1]=iso;
    E->f[0]=face;
    edgepos++;   
    return 0;
  } else {
    // muuten verrataan onko sama edge
    if (E->v[0]==pieni && E->v[1]==iso) {
      // jos on sama edge niin kakkosface talteen ja eteenp‰in
      E->f[1]=face;
      //NEIGHBOURS!!1
      return 0;
    } else {
      // ei ole sama edge joten katotaan onko next m‰‰ritelty, jos ei ole niin kaikki on k‰yty l‰pi eik‰ lˆytynyt, joten talletetaan edge ja siirryt‰‰n seuraavaan
      if (E->next==0) {
        edge *EN;
        
        E->next=malloc(sizeof(edge));

        EN=E->next;
        EN->v[0]=pieni;
        EN->v[1]=iso;
        EN->f[0]=face;
        EN->f[1]=-1;
        EN->next=0;
        EN->sharpness=0;
        edgepos++;
        return 0;
      } else {
        // muuten menn‰‰n seuraavaan edgeen linked-listiss‰ ja tehd‰‰n samat temput
        handle_edge(E->next, pieni, iso, face);
      }
    }
  }
  
  return 0;
}


/********************************
********  file reading  *********
********************************/

int getln(char *dest) {
  char temp[1000];
  int x=datapos;

  memset(temp, 0, 1000);

  while(tempbuf[datapos]!=10) {
    temp[datapos-x]=tempbuf[datapos];
    datapos++;
    if (datapos>=size) return 0;
  }
  
  sprintf(dest, "%s", temp);
  datapos++;
  return 1;
}


void substr(char *sourke, char *dest, int len) {
  int x;
  
  for (x=0; x<len; x++) {
    dest[x]=sourke[x];
  }

  dest[len]=0;
}



/**********************************************
********  multiple texcoord-shitfuck  *********
**********************************************/

typedef struct {
  int vi, ti, ni;
  int newind;
} hasa;

hasa hasataulu[MAX_VERTS*MAX_ATTACH];


int hashcheck(int vi, int ti, int ni) {
  int x;
  
  for (x=0; x<MAX_ATTACH; x++) {
    int i=vi*MAX_ATTACH+x;
        
//  printf("vi=%i, ti=%i, hashvi=%i, hashti=%i", vi, ti, hasataulu[i].vi, hasataulu[i].ti);
//  getch();
    
    if (hasataulu[i].vi==vi && hasataulu[i].ti==ti && hasataulu[i].ni==ni) {
//    printf("BINGO\n");
      return hasataulu[i].newind;
    }
    
    if (hasataulu[i].vi==-1) {
      
//    printf("NU\n"); 
      
      hasataulu[i].vi=vi;
      hasataulu[i].ti=ti;
      hasataulu[i].ni=ni;
      hasataulu[i].newind=vertexpos;
      
      if (x>0) {
        muthas[muthapos+0]=vertexpos;
        muthas[muthapos+1]=hasataulu[vi*MAX_ATTACH].newind;
        muthapos+=2;
        vertices_splitted++;
      }

      vertexpos++;

      return hasataulu[i].newind;
    }
  }
  
  return -666;
}


int nu_vertex(int vi, int ti, int ni) {
  int plaa;
  
  plaa=hashcheck(vi, ti, ni);
  if (plaa==-666) {
    printf("\n\nERREUR: out of hash!\n\n");
    exit(0);    
  }    
  
  temp_vlist[plaa]=temp_vlist_x[vi];
  temp_tlist[plaa]=temp_tlist_x[ti];
  temp_nlist[plaa]=temp_nlist_x[ni];
  
  if (vertexpos>MAX_VERTS-1) {
    printf("\n\nERREUR: out of vertexbuffer\n\n");
    exit(0);    
  }    
  
  return plaa;
}

int filelength(FILE *f) {
  fseek(f, 0, SEEK_END);
  int sz = ftell(f);
  fseek(f, 0, SEEK_SET);
  return sz;
}

int main(int argc, char *argv[]) {
  FILE *in;
  FILE *out;
  char rivi[1000];
  char subrivi[1000];
  int x;
  int maxface=0;
  char name[100], infile[100], outfile[100];
  //char progress[]={'|','/','-','\'};

  /* parametrit */
  if (argc<2) {
    printf("\n\nUsage: obj2inc.exe object_name");
    printf("\n   -i   intro size");
    //printf("\n   -t   read texture coordinates");
    printf("\n   -n   calculate neighbours");
    printf("\n   -f   flip y/z axis");
    printf("\n   -no  do not optimize");
    printf("\n   -e   find edges");
    printf("\n   -b   binary output");
    printf("\n   -l   flat");    
    printf("\n\n");
    exit(0);
  }

  for (x=2; x<argc; x++) {
    if (strcmp(argv[x], "-i")==0) {
      introkoko=1;
      printf("\n\nDoing it introsize\n");
    }
/*
    if (strcmp(argv[x], "-t")==0) {
      mapped=1;
      printf("\n\nHandling texturecoordinates\n");
    }
*/
    if (strcmp(argv[x], "-n")==0) {
      neighbours=1;
      printf("\n\nCalculating neighbours\n");
    }

    if (strcmp(argv[x], "-f")==0) {
      yzflip=1;
      printf("\n\nFlipping y- and z-axis");
    }

    if (strcmp(argv[x], "-no")==0) {
      optimize=0;
      printf("\n\nSkipping optimization");
    }

    if (strcmp(argv[x], "-l")==0) {
      flat=1;
      optimize=0;
      printf("\n\nFlat shade");
    }
    
    if (strcmp(argv[x], "-e")==0) {
      find_edges=1;
      printf("\n\nSearching edges");
    }

    if (strcmp(argv[x], "-b")==0) {
      binary=1;
      printf("\n\nOutputting binary format (atm only vertices/incides/texcoords)"); 
    }    


  }

  /* filen lukeminen */

  sprintf(name, argv[1]);
  sprintf(infile, "%s.obj", name);
  if (binary) {
    sprintf(outfile, "%s.bin", name);  
  } else {
    sprintf(outfile, "%s.inc", name);  
  }

  in=fopen(infile, "rb");
  if (binary) out=fopen(outfile, "wb"); 
  else out=fopen(outfile, "w");
  
  if (in==0 || out==0) {
    printf("\n\nFile error!\n\n");
    exit(0);
  } else {
    printf("\n\nReading from %s\n", infile);
  }

  size=filelength(in);
  tempbuf=malloc(size);
  fread(tempbuf, size, 1, in);
  fclose(in);

  printf("\nFile read, size %i\n", size);

  temp_vlist_x=malloc(sizeof(vertex)*MAX_VERTS);
  temp_vlist=malloc(sizeof(vertex)*MAX_VERTS);  
  temp_flist=malloc(sizeof(face)*MAX_FACES);
  temp_flist2=malloc(sizeof(face)*MAX_FACES);
  temp_tlist=malloc(sizeof(vertex)*MAX_VERTS);
  temp_tlist_x=malloc(sizeof(vertex)*MAX_VERTS);
  temp_nlist=malloc(sizeof(vertex)*MAX_VERTS);
  temp_nlist_x=malloc(sizeof(vertex)*MAX_VERTS);
  if (find_edges) temp_elist=malloc(sizeof(edge)*MAX_EDGES); 
  
  orig_flist=malloc(sizeof(face)*MAX_FACES);
  muthas=malloc(sizeof(int)*MAX_FACES*2);
  

  for (x=0; x<MAX_VERTS*MAX_ATTACH; x++) hasataulu[x].vi=hasataulu[x].ti=-1;

  while (1) {
    
    if (!getln(rivi)) break;
    substr(rivi, subrivi, 2);
    
    // verteksi:
    //v -0.036499 0.093185 -1.356277
    if (strcmp(subrivi, "v ")==0) {
      float xx=0, yy=0, zz=0;

      sscanf(rivi, "v %f %f %f", &xx, &yy, &zz);

      if (yzflip) {
        temp_vlist_x[vertexpos_x].x=-xx;
        temp_vlist_x[vertexpos_x].y=zz;
        temp_vlist_x[vertexpos_x].z=yy;
      } else {
        temp_vlist_x[vertexpos_x].x=xx;
        temp_vlist_x[vertexpos_x].y=yy;
        temp_vlist_x[vertexpos_x].z=zz;
      }
      vertexpos_x++;
    } else if (strcmp(subrivi, "vt")==0) {
      // texturekoordinaatti
      //vt 0.414063 0.800000
      float uu=0, vv=0;

      sscanf(rivi, "vt %f %f", &uu, &vv);
      
      temp_tlist_x[texturepos_x].x=uu;
      temp_tlist_x[texturepos_x].y=vv;
      temp_tlist_x[texturepos_x].z=0;      
      texturepos_x++;

    } else if (strcmp(subrivi, "vn") == 0) {
      // normal
      float nx=0, ny=0, nz=0;

      sscanf(rivi, "vn %f %f %f", &nx, &ny, &nz);
      temp_nlist_x[normalpos_x].x = nx;
      temp_nlist_x[normalpos_x].y = ny;
      temp_nlist_x[normalpos_x].z = nz;
      normalpos_x++;

    } else if (strcmp(subrivi, "f ")==0) {
      // face
      //f 955/955 956/956 1085/1085    
      static int v1, v2, v3;
      static int t1, t2, t3;
      static int vv1, vv2, vv3;
      static int n1, n2, n3;


      if (texturepos_x && normalpos_x) { // texturecoords+normals 
        sscanf(rivi, "f %i/%i/%i %i/%i/%i %i/%i/%i", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
      } else if (texturepos_x) { // texcoords
        sscanf(rivi, "f %i/%i %i/%i %i/%i", &v1, &t1, &v2, &t2, &v3, &t3);
      } else if (normalpos_x) { // normalz
        sscanf(rivi, "f %i//%i %i//%i %i//%i", &v1, &n1, &v2, &n2, &v3, &n3);
      } else { // just vertices
        sscanf(rivi, "f %i %i %i", &v1, &v2, &v3);
      }
/*

      if (mapped) sscanf(rivi, "f %i/%i %i/%i %i/%i", &v1, &t1, &v2, &t2, &v3, &t3);
      else {
        sscanf(rivi, "f %i %i %i", &v1, &v2, &v3);
        t1=t2=t3=1;
      }
      
      if (v1>maxface) maxface=v1;
      if (v2>maxface) maxface=v2;
      if (v3>maxface) maxface=v3;
*/
      /*
      if (mapped) {
        if ((v1!=t1) || (v2!=t2) || (v3!=t3)) {
          printf("\n\nERREUR: Texcoord and vertex index mismatch!!1\n\n");
          exit(0);
        }
      }
      */

      vv1=nu_vertex(v1-1, t1-1, n1-1);
      vv2=nu_vertex(v2-1, t2-1, n2-1);
      vv3=nu_vertex(v3-1, t3-1, n3-1);  
    
      orig_flist[orig_fpos].v[0]=hasataulu[(v1-1)*MAX_ATTACH].newind;
      orig_flist[orig_fpos].v[1]=hasataulu[(v2-1)*MAX_ATTACH].newind;
      orig_flist[orig_fpos].v[2]=hasataulu[(v3-1)*MAX_ATTACH].newind;
      orig_fpos++;

      temp_flist[facepos].v[0]=vv1;
      temp_flist[facepos].v[1]=vv2;
      temp_flist[facepos].v[2]=vv3;
      temp_flist[facepos].used=0;
      temp_flist2[facepos].n[0]=-1;
      temp_flist2[facepos].n[1]=-1;
      temp_flist2[facepos].n[2]=-1;
      
      facepos++;      
    } 
    
    printf("\rV:%i, T:%i, F:%i N:%i", vertexpos_x, texturepos_x, facepos, normalpos_x);
  }
  

  if (vertices_splitted && !flat) {
    printf("\n\nPAY ATTENTION YOU DUMB FUCK!\nHad to split vertices due to multiple texcooords per vertex,\nwriting out old facestruct and links between new and old vertices\n");
  }



/*
  if (mapped && (texturepos != vertexpos)) {
    printf("\n\nERREUR: texturepos != vertexpos\n\n");
    exit(0);
  }

  if (maxface != vertexpos) {
    printf("\n\nERREUR: maxface (%i) != vertexpos (%i)\n\n", maxface, vertexpos);
    exit(0);
  }
*/
  
  
  // optimizing stupfh
  if (optimize) {
    printf("\nOptimizing...\n");
 
    clear_cache();

    for (x=0; x<facepos; x++) {
      int hits;

      hits=cache_hits(x, 0);
      bhits+=hits;
      bmisses+=3-hits;
      draw_face(x, 0);
    }

    clear_cache();

    for (x=0; x<facepos; x++) {
      next_face();      
      printf("%i..%i\r", x, facepos);
    }

    clear_cache();

    for (x=0; x<facepos; x++) {
      int hits;

      hits=cache_hits(x, 1);
      ahits+=hits;
      amisses+=3-hits;
      draw_face(x, 1);
    }

    printf("\n\nvertex cache size %i\n\n", VCACHE);
    printf("vertex cache hits (before): %i\n", bhits);
    printf("vertex cache hits (after): %i\n", ahits);
    printf("vertex cache misses (before): %i\n", bmisses);
    printf("vertex cache misses (after): %i\n", amisses);

    printf("cache misses per triangle (acmr): 0.5 - 3.0\n\n");
    printf("perfect acmr: %5.2f\n", (float)vertexpos/(float)facepos);
    printf("acmr before: %5.2f\n", (float)bmisses/(float)facepos);
    printf("acmr after: %5.2f\n", (float)amisses/(float)facepos);
  } else { // ei optimointia
    for (x=0; x<facepos; x++) {
      temp_flist2[x].v[0]=temp_flist[x].v[0];
      temp_flist2[x].v[1]=temp_flist[x].v[1];
      temp_flist2[x].v[2]=temp_flist[x].v[2];
    }      
  }

  
  // edge stuff
  if (find_edges) {
    for (x=0; x<vertexpos; x++) {
      temp_elist[x].v[0]=-1; 
      temp_elist[x].v[1]=-1;
      temp_elist[x].f[0]=-1; 
      temp_elist[x].f[1]=-1;
      temp_elist[x].next=0;
      temp_elist[x].sharpness=0;
    }
    
    for (x=0; x<facepos; x++) {
      int pieni, iso, swap;
      int e;
      
      for (e=0; e<3; e++) {
        pieni=temp_flist2[x].v[e];
        iso=temp_flist2[x].v[(e+1)%3];
                
        if (pieni>iso) {
          swap=pieni; pieni=iso; iso=swap;
        }
        
        handle_edge(&temp_elist[pieni], pieni, iso, x);        
      }
    }
  }  
  
  // etit‰‰n naapurit
  if (neighbours) {
    int x, xx, y, yy;
    int fc=facepos;
   
    for (x=0; x<fc; x++) for (xx=0; xx<3; xx++) {   
      for (y=0; y<fc; y++) for (yy=0; yy<3; yy++) {

        if
          (
            (
              (temp_flist2[x].v[(xx+0)%3]==temp_flist2[y].v[(yy+0)%3] &&
               temp_flist2[x].v[(xx+1)%3]==temp_flist2[y].v[(yy+1)%3])
              ||
              (temp_flist2[x].v[(xx+1)%3]==temp_flist2[y].v[(yy+0)%3] &&
               temp_flist2[x].v[(xx+0)%3]==temp_flist2[y].v[(yy+1)%3])
            )
          && x!=y
        )
        temp_flist2[x].n[xx]=y;
      }
    }
  }


  //  tiedoston kirjoitus
  printf("\n\nWriting to %s\n", outfile);



  if (binary) {
  
    printf("\n%i vertices", vertexpos);
    printf("\n%i faces", facepos);
  
    fwrite(&vertexpos, 1, 4, out);
    fwrite(&facepos, 1, 4, out);
       
    for (x=0; x<vertexpos; x++) {    
      fwrite(&temp_vlist[x].x, 1, 4, out);
      fwrite(&temp_vlist[x].y, 1, 4, out);
      fwrite(&temp_vlist[x].z, 1, 4, out);      
    }    

    for (x=0; x<vertexpos; x++) {
      fwrite(&temp_tlist[x].x, 1, 4, out);
      fwrite(&temp_tlist[x].y, 1, 4, out);  
   }
   
    for (x=0; x<facepos; x++) {  
      fwrite(&temp_flist2[x].v[0], 1, 4, out);
      fwrite(&temp_flist2[x].v[1], 1, 4, out);
      fwrite(&temp_flist2[x].v[2], 1, 4, out);
    }      
           
     
  } else if (flat) {

    fprintf(out, "int %s_vc=%i;\n", name, facepos*3);
    fprintf(out, "int %s_fc=%i;\n", name, facepos);
  
    printf("Vertices\n");
    fprintf(out, "static float %s_vertices[]={\n", name);
    for (x=0; x<facepos; x++) {
      int i1, i2, i3;
      i1=temp_flist2[x].v[0];
      i2=temp_flist2[x].v[1];
      i3=temp_flist2[x].v[2];
      fprintf(out, "/* %i */  %15.7f, %15.7f, %15.7f,\n", x*3+0, temp_vlist[i1].x, temp_vlist[i1].y, temp_vlist[i1].z);
      fprintf(out, "/* %i */  %15.7f, %15.7f, %15.7f,\n", x*3+1, temp_vlist[i2].x, temp_vlist[i2].y, temp_vlist[i2].z);
      fprintf(out, "/* %i */  %15.7f, %15.7f, %15.7f,\n", x*3+2, temp_vlist[i3].x, temp_vlist[i3].y, temp_vlist[i3].z);
    }
    fprintf(out, "};\n\n");
  
    printf("Faces\n");
    fprintf(out, "static unsigned short %s_indices[]={\n", name);
    for (x=0; x<facepos; x++) {
      fprintf(out, "/* %i */  %i, %i, %i,\n", x, x*3, x*3+1, x*3+2);
    }
    fprintf(out, "};\n\n");
  
  } else {
    
    // kirjotetaan muuttujat
    fprintf(out, "int %s_vc=%i;\n", name, vertexpos);
    fprintf(out, "int %s_fc=%i;\n", name, facepos);
    fprintf(out, "int %s_ec=%i;\n", name, edgepos);
    fprintf(out, "int %s_linkcnt=%i;\n", name, muthapos/2);
    fprintf(out, "int %s_orig_vc=%i;\n", name, vertexpos_x);
  
//    fprintf(out, "Object *%s;\n\n", name);
    
    printf("Vertices\n");
    fprintf(out, "static float %s_vertices[]={\n", name);
    for (x=0; x<vertexpos-1; x++) {
      if (introkoko) {
        fprintf(out, "/* %i */  %15.f/4., %15.f/4., %15.f/4.,\n", x, temp_vlist[x].x*4, temp_vlist[x].y*4, temp_vlist[x].z*4);
      } else { 
        fprintf(out, "/* %i */  %15.7f, %15.7f, %15.7f,\n", x, temp_vlist[x].x, temp_vlist[x].y, temp_vlist[x].z);
      }
    }
    if (introkoko) {
      fprintf(out, "/* %i */  %15.f/4., %15.f/4., %15.f/4.};\n\n", vertexpos-1, temp_vlist[vertexpos-1].x*4, temp_vlist[vertexpos-1].y*4, temp_vlist[vertexpos-1].z*4);
    } else {
      fprintf(out, "/* %i */  %15.7f, %15.7f, %15.7f};\n\n", vertexpos-1, temp_vlist[vertexpos-1].x, temp_vlist[vertexpos-1].y, temp_vlist[vertexpos-1].z);
    } 
  
    if (texturepos_x) {
      printf("Texture coordinates\n");
      fprintf(out, "static float %s_texcoords[]={\n", name);
      for (x=0; x<vertexpos-1; x++) {
        fprintf(out, "/* %i */  %15.7f, %15.7f,\n", x, temp_tlist[x].x, temp_tlist[x].y);
      }
      fprintf(out, "/* %i */  %15.7f, %15.7f};\n\n", vertexpos-1, temp_tlist[vertexpos-1].x, temp_tlist[vertexpos-1].y);
    }
  
    if (normalpos_x) {      
      printf("Normals\n");
      fprintf(out, "static float %s_normals[]={\n", name);
      for (x=0; x<vertexpos-1; x++) {
        fprintf(out, "/* %i */  %15.7f, %15.7f, %15.7f,\n", x, temp_nlist[x].x, temp_nlist[x].y, temp_nlist[x].z);
      }
      fprintf(out, "/* %i */  %15.7f, %15.7f, %15.7f};\n\n", vertexpos-1, temp_nlist[vertexpos-1].x, temp_nlist[vertexpos-1].y, temp_nlist[vertexpos-1].z);
    }
  
    printf("Faces\n");
    fprintf(out, "static unsigned short %s_indices[]={\n", name);
    for (x=0; x<facepos-1; x++) {
      fprintf(out, "/* %i */  %i, %i, %i,\n", x, temp_flist2[x].v[0], temp_flist2[x].v[1], temp_flist2[x].v[2]);
    }
    fprintf(out, "/* %i */  %i, %i, %i};\n\n", facepos-1, temp_flist2[facepos-1].v[0], temp_flist2[facepos-1].v[1], temp_flist2[facepos-1].v[2]);
  
  
    if (neighbours) {
  //    int x, xx, y, yy;
  //    int fc=facepos;
     
      printf("Neighbours\n");
      fprintf(out, "static int %s_neighbours[]={\n", name);
      for (x=0; x<facepos-1; x++) {
        fprintf(out, "/* %i */  %i, %i, %i,\n", x, temp_flist[x].n[0], temp_flist[x].n[1], temp_flist[x].n[2]);
      }
      fprintf(out, "/* %i */  %i, %i, %i};\n\n", facepos-1, temp_flist[facepos-1].n[0], temp_flist[facepos-1].n[1], temp_flist[facepos-1].n[2]);
    }
  
  
    if (find_edges) {
      edge *E;
      int e2=0;
      
      printf("Edges\n");
  
      fprintf(out, "static int %s_edges[]={\n", name);
      for (x=0; x<vertexpos; x++) {
  
        E=&temp_elist[x];      
  
        while (E!=0) {        
          if (E->v[0]!=-1) {
            fprintf(out, "  %i, %i, %i, %i,", E->v[0], E->v[1], E->f[0], E->f[1]);
            
            e2++;
  
            if (e2==edgepos) fprintf(out, "};\n\n");
            else fprintf(out, ",\n");
          }
  
          E=E->next;
        }
      }
    }
  
  
  
    if (vertices_splitted) {
  
      printf("Original Faces\n");
      fprintf(out, "static int %s_orig_indices[]={\n", name);
      for (x=0; x<orig_fpos; x++) {
        fprintf(out, "/* %i */  %i, %i, %i,\n", x, orig_flist[x].v[0], orig_flist[x].v[1], orig_flist[x].v[2]);
      }
      fprintf(out, "};\n\n");
  
      printf("Vertex links\n");
      fprintf(out, "static int %s_vertex_links[]={\n", name);
      for (x=0; x<muthapos; x+=2) {
        fprintf(out, "  %i, %i,\n", muthas[x], muthas[x+1]);
      } 
      fprintf(out, "};\n\n");
    }
  }

  fclose(out);
  /*
  free(temp_vlist); 
  free(temp_flist);
  free(temp_tlist);
  free(temp_nlist);
  free(temp_nlist_x);
  if (find_edges) free(temp_elist);

  ...
   temp_vlist_x=malloc(sizeof(vertex)*MAX_VERTS);
  temp_vlist=malloc(sizeof(vertex)*MAX_VERTS);  
  temp_flist=malloc(sizeof(face)*MAX_FACES);
  temp_flist2=malloc(sizeof(face)*MAX_FACES);
  temp_tlist=malloc(sizeof(vertex)*MAX_VERTS);
  temp_tlist_x=malloc(sizeof(vertex)*MAX_VERTS);
  temp_nlist=malloc(sizeof(vertex)*MAX_VERTS);
  temp_nlist_x=malloc(sizeof(vertex)*MAX_VERTS);
  if (find_edges) temp_elist=malloc(sizeof(edge)*MAX_EDGES); 
  
  orig_flist=malloc(sizeof(face)*MAX_FACES);
  muthas=malloc(sizeof(int)*MAX_FACES*2);
 
*/
  return 0;
}
