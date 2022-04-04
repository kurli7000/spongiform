

Mesh *cubemesh(int numcubes) {
  float *v = malloc(sizeof(float)*numcubes*24*6);
  unsigned short *i = malloc(sizeof(unsigned short)*numcubes*24);
  CubeMesh *m = malloc(sizeof(Mesh));
  int x;
  
  memset(v, 0, sizeof(float)*numcubes*24*6);
  if (numcubes*24>65535) LOGE("too many cubes dumbass");

  glGenBuffers(1, &m->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m->vbo);  
  glBufferData(GL_ARRAY_BUFFER, numcubes*24*6*sizeof(float), v, GL_DYNAMIC_DRAW);

  for (x=0; x<numcubes*36; x++) i[x]=x;

  glGenBuffers(1, &m->ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*numcubes*36, indices, GL_STATIC_DRAW);

  m->vc = numcubes*24;
  m->fc = numcubes*36;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  m->vertexdata = v;
  m->TC = false;

  free(i);
  return m;
}


void drawcubes(CubeMesh *m, float *posses, float *dirs) {
  int x;
  int poshandle, texhandle, normhandle;
  int stride = 6;


  glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);

  for (x=0; x<m->vc; x++) {
    m->vertexdata[x] = ...; puukot tähän
  }

  glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned short)*m->vc*6, tmpbuffer, GL_DYNAMIC_DRAW);

  poshandle = glGetAttribLocation(currentshader, POSITION_ARRAY_NAME);
  glVertexAttribPointer(poshandle, 3, GL_FLOAT, GL_FALSE, sizeof(float)*stride, (GLvoid*)NULL);
  glEnableVertexAttribArray(poshandle);

  normhandle = glGetAttribLocation(currentshader, NORMAL_ARRAY_NAME);
  glVertexAttribPointer(normhandle, 3, GL_FLOAT, GL_FALSE, sizeof(float)*stride, (GLvoid*)(sizeof(float)*3));
  glEnableVertexAttribArray(normhandle);  

  glDrawElements(GL_TRIANGLES, m->fc, GL_UNSIGNED_SHORT, (GLvoid*)NULL);

  glDisableVertexAttribArray(poshandle);
  glDisableVertexAttribArray(normhandle);
  //glDisableVertexAttribArray(texhandle);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  checkerror("drawcubes");
}


void draw_cubes_dirzzz(Vertex *pos, Vertex *dir, int vc, Vertex size, int norm, int flags) {
  void *tmpptr=tmpmalloc(0);
  Vertex *v, *t, *n;
  int *i;
  int x;
  float w1=size.x, w2=size.z, l=size.y*2;
  Vertex tt[4];
  Vertex yv=new_v(1, 0, 0);

  v=tmpmalloc(sizeof(Vertex)*vc*24);
  t=tmpmalloc(sizeof(Vertex)*vc*24);
  n=tmpmalloc(sizeof(Vertex)*vc*24);
  i=tmpmalloc(sizeof(int)*vc*24);

  tt[0]=new_v(0, 0, 0);
  tt[1]=new_v(1, 0, 0);
  tt[2]=new_v(1, 1, 0);
  tt[3]=new_v(0, 1, 0);

  if (norm) for (x=0; x<vc; x++) normalize(dir+x);


  for (x=0; x<vc; x++) {
    Vertex d1=cross(dir+x, &yv);
    Vertex d2=cross(&d1, dir+x);
    Vertex *p=pos+x;
    Vertex v1, v2, v3, v4, v5, v6, v7, v8;
    Vertex *d=dir+x;
    int y;

    if (norm) {
      normalize(&d1);
      normalize(&d2);
    }

    v1=new_v(p->x - d1.x*w1 - d2.x*w2, p->y - d1.y*w1 - d2.y*w2, p->z - d1.z*w1 - d2.z*w2);
    v2=new_v(p->x + d1.x*w1 - d2.x*w2, p->y + d1.y*w1 - d2.y*w2, p->z + d1.z*w1 - d2.z*w2);
    v3=new_v(p->x + d1.x*w1 + d2.x*w2, p->y + d1.y*w1 + d2.y*w2, p->z + d1.z*w1 + d2.z*w2);
    v4=new_v(p->x - d1.x*w1 + d2.x*w2, p->y - d1.y*w1 + d2.y*w2, p->z - d1.z*w1 + d2.z*w2);

    v5=new_v(v1.x + d->x*l, v1.y + d->y*l, v1.z + d->z*l);
    v6=new_v(v2.x + d->x*l, v2.y + d->y*l, v2.z + d->z*l);
    v7=new_v(v3.x + d->x*l, v3.y + d->y*l, v3.z + d->z*l);
    v8=new_v(v4.x + d->x*l, v4.y + d->y*l, v4.z + d->z*l);

    v[x*24+ 0]=v1; v[x*24+ 1]=v2; v[x*24+ 2]=v3; v[x*24+ 3]=v4;
    v[x*24+ 4]=v8; v[x*24+ 5]=v7; v[x*24+ 6]=v6; v[x*24+ 7]=v5;
    v[x*24+ 8]=v1; v[x*24+ 9]=v5; v[x*24+10]=v6; v[x*24+11]=v2;
    v[x*24+12]=v2; v[x*24+13]=v6; v[x*24+14]=v7; v[x*24+15]=v3;
    v[x*24+16]=v3; v[x*24+17]=v7; v[x*24+18]=v8; v[x*24+19]=v4;
    v[x*24+20]=v4; v[x*24+21]=v8; v[x*24+22]=v5; v[x*24+23]=v1;

    n[x*24+ 3]=n[x*24+ 2]=n[x*24+ 1]=n[x*24+ 0]=new_v(-d->x, -d->y, -d->z);
    n[x*24+ 7]=n[x*24+ 6]=n[x*24+ 5]=n[x*24+ 4]=d[0];
    n[x*24+11]=n[x*24+10]=n[x*24+ 9]=n[x*24+ 8]=new_v(-d2.x, -d2.y, -d2.z);
    n[x*24+15]=n[x*24+14]=n[x*24+13]=n[x*24+12]=d1;
    n[x*24+19]=n[x*24+18]=n[x*24+17]=n[x*24+16]=d2;
    n[x*24+23]=n[x*24+22]=n[x*24+21]=n[x*24+20]=new_v(-d1.x, -d1.y, -d1.z);

    for (y=0; y<24; y++) {
      t[x*24+y]=tt[y&3];
      i[x*24+y]=x*24+y;
    }
  }


  renderflags(flags);

  glue_disableallarrays();

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib(GL_ALL_CLIENT_ATTRIB_BITS);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, v);

  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, 0, n);

  glClientActiveTexture(GL_TEXTURE1_ARB);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(3, GL_FLOAT, 0, t);

  glClientActiveTexture(GL_TEXTURE0_ARB);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(3, GL_FLOAT, 0, t);

  glDrawElements(GL_QUADS, vc*24, GL_UNSIGNED_INT, i);

  glPopClientAttrib();
  glPopAttrib();

  glueFC+=vc*24*2;
  glueVC+=vc*24;


  tmpfree(tmpptr);

}



void draw_cubes_dir(Vertex *pos, Vertex *dir, int vc, Vertex size, int norm, int flags) {
  int i;
  int bsize = 128;

  for (i = 0; i<vc; i+=bsize) draw_cubes_dirzzz(pos+i, dir+i, vc-i<bsize ? vc-i : bsize, size, norm, flags);
}



void draw_cubes_gridzzz(Vertex *pos, Vertex *size, Vertex *texc, int vc, int onetu, int flags) {
  void *tmpptr=tmpmalloc(0);
  Vertex *v, *t, *n, *t2=0;
  int *i;
  int x;
  Vertex tt[4];
  Vertex d1=new_v(1, 0, 0);
  Vertex d2=new_v(0, 0, 1);
  Vertex d=new_v(0, 1, 0);

  v=tmpmalloc(sizeof(Vertex)*vc*24);
  t=tmpmalloc(sizeof(Vertex)*vc*24);
  if (texc) t2=tmpmalloc(sizeof(Vertex)*vc*24);
  n=tmpmalloc(sizeof(Vertex)*vc*24);
  i=tmpmalloc(sizeof(int)*vc*24);

  tt[0]=new_v(0, 0, 0);
  tt[1]=new_v(1, 0, 0);
  tt[2]=new_v(1, 1, 0);
  tt[3]=new_v(0, 1, 0);

  for (x=0; x<vc; x++) {
    Vertex *p=pos+x;
    Vertex v1, v2, v3, v4, v5, v6, v7, v8;
    int y;
    float w1=size[x].x, w2=size[x].z, l=size[x].y;

    v1=new_v(p->x - w1, p->y - l, p->z - w2);
    v2=new_v(p->x + w1, p->y - l, p->z - w2);
    v3=new_v(p->x + w1, p->y - l, p->z + w2);
    v4=new_v(p->x - w1, p->y - l, p->z + w2);

    v5=new_v(p->x - w1, p->y + l, p->z - w2);
    v6=new_v(p->x + w1, p->y + l, p->z - w2);
    v7=new_v(p->x + w1, p->y + l, p->z + w2);
    v8=new_v(p->x - w1, p->y + l, p->z + w2);

    v[x*24+ 0]=v1; v[x*24+ 1]=v2; v[x*24+ 2]=v3; v[x*24+ 3]=v4;
    v[x*24+ 4]=v8; v[x*24+ 5]=v7; v[x*24+ 6]=v6; v[x*24+ 7]=v5;
    v[x*24+ 8]=v1; v[x*24+ 9]=v5; v[x*24+10]=v6; v[x*24+11]=v2;
    v[x*24+12]=v2; v[x*24+13]=v6; v[x*24+14]=v7; v[x*24+15]=v3;
    v[x*24+16]=v3; v[x*24+17]=v7; v[x*24+18]=v8; v[x*24+19]=v4;
    v[x*24+20]=v4; v[x*24+21]=v8; v[x*24+22]=v5; v[x*24+23]=v1;

    n[x*24+ 3]=n[x*24+ 2]=n[x*24+ 1]=n[x*24+ 0]=new_v(-d.x, -d.y, -d.z);
    n[x*24+ 7]=n[x*24+ 6]=n[x*24+ 5]=n[x*24+ 4]=d;
    n[x*24+11]=n[x*24+10]=n[x*24+ 9]=n[x*24+ 8]=new_v(-d2.x, -d2.y, -d2.z);
    n[x*24+15]=n[x*24+14]=n[x*24+13]=n[x*24+12]=d1;
    n[x*24+19]=n[x*24+18]=n[x*24+17]=n[x*24+16]=d2;
    n[x*24+23]=n[x*24+22]=n[x*24+21]=n[x*24+20]=new_v(-d1.x, -d1.y, -d1.z);

    if (texc) for (y=0; y<24; y++) t2[x*24+y]=texc[x];
    for (y=0; y<24; y++) t[x*24+y]=tt[y&3];
    for (y=0; y<24; y++) i[x*24+y]=x*24+y;

  }


  renderflags(flags);
  //glEnable(GL_ALPHA_TEST);
  //glAlphaFunc(GL_GREATER, 0);

  glue_disableallarrays();

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib(GL_ALL_CLIENT_ATTRIB_BITS);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, v);

  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, 0, n);


  if (onetu) {
    if (texc) {
      glClientActiveTexture(GL_TEXTURE0_ARB);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(3, GL_FLOAT, 0, t2);
    } else {
      glClientActiveTexture(GL_TEXTURE0_ARB);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(3, GL_FLOAT, 0, t);
    }
  } else {
    if (texc) {
      glClientActiveTexture(GL_TEXTURE1_ARB);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(3, GL_FLOAT, 0, t2);
    }

    glClientActiveTexture(GL_TEXTURE0_ARB);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(3, GL_FLOAT, 0, t);
  }

  glDrawElements(GL_QUADS, vc*24, GL_UNSIGNED_INT, i);

  glPopClientAttrib();
  glPopAttrib();

  glueFC+=vc*24*2;
  glueVC+=vc*24;


  tmpfree(tmpptr);

}


void draw_cubes_grid(Vertex *pos, Vertex *size, Vertex *texc, int vc, int onetu, int flags) {
  int i;
  int bsize = 128;

  for (i = 0; i<vc; i+=bsize) draw_cubes_gridzzz(pos+i, size+i, texc?texc+i:0, vc-i<bsize ? vc-i : bsize, onetu, flags);
}

