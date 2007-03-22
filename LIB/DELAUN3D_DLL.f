c     ------   subroutine input   -----------------------------------
c
c     purpose : input of data
c     last modified :  5 sep 1989
c
      subroutine input(ierr,ktj,node,x,y,z,fname)
	!DEC$ ATTRIBUTES DLLEXPORT :: INPUT
      implicit real*8(a-h,o-z)
      dimension x(ktj+4),y(ktj+4),z(ktj+4)
      character fname*30

      ierr = -1
      
      open(8,file=fname)
      read(8,*)node
      read(8,*)(x(i),y(i),z(i),i=1,node)
      close(8)

      ierr = 0

      return
      end

c
c     ------   subroutine data   ---------------------------------------
c
c     purpose : print results on data file
c     last modified : 20 oct 1990
c
      subroutine output(ierr,kte,ktj,nelm,node,mtj,jac,x,y,z,fname)
	!DEC$ ATTRIBUTES DLLEXPORT :: OUTPUT
      implicit real*8(a-h,o-z)
      dimension mtj(kte,4),jac(kte,4),x(ktj+4),y(ktj+4),z(ktj+4)
      character fname*30

      ierr = -1
      
	open(9,file=fname)
	  write(9,600)nelm,node
	  write(9,610)((mtj(i,j),j=1,4),(jac(i,j),j=1,4),i=1,nelm)
	  write(9,620)(x(i),y(i),z(i),i=1,node)
	close(9)

  600 format(2i8)
  610 format(8i8)
  620 format(3d15.7)
      
      ierr = 0
      
      return
      end
c
c     ------   subroutine tetgen   -------------------------------------
c
c     purpose : the delaunay triangulation in 3-dimensional space
c     last modified : 18 mar 1991  c ohta
c
c     max points : 5000
c     max elements : 20000
c
      subroutine tetgen(ierrcode,node,x,y,z,nelm,mtj,jac,ktj,kte)
	!DEC$ ATTRIBUTES DLLEXPORT :: TETGEN
      implicit real*8(a-h,o-z)
      parameter(err=1.0d-12)
c      parameter(ktj=5000,kte=20000,err=1.0d-12)
c      dimension x(ktj+4),y(ktj+4),z(ktj+4),list(ktj),ibin(ktj)
      dimension x(ktj+4),y(ktj+4),z(ktj+4),list(ktj),ibin(ktj)
      dimension mtj(kte,4),jac(kte,4),vx(kte),vy(kte),vz(kte),rv(kte)
      dimension kv(kte),istack(kte),map(kte)
      
c      WRITE(*,*) 'START TETGEN()'
      
	ierrcode = 0
c
c     computation of max & min coords for x,y,z
c
      xmin=x(1)
      xmax=xmin
      ymin=y(1)
      ymax=ymin
      zmin=z(1)
      zmax=zmin
      do 10 i=2,node
	  xmin=dmin1(xmin,x(i))
	  xmax=dmax1(xmax,x(i))
	  ymin=dmin1(ymin,y(i))
	  ymax=dmax1(ymax,y(i))
	  zmin=dmin1(zmin,z(i))
	  zmax=dmax1(zmax,z(i))
   10 continue
      rax=xmax-xmin
      ray=ymax-ymin
      raz=zmax-zmin
      dmax=dmax1(rax,ray,raz)
c
c     normalization of x,y,z-coords of points
c
      do 20 i=1,node
      	x(i)=(x(i)-xmin)/dmax
	  y(i)=(y(i)-ymin)/dmax
	  z(i)=(z(i)-zmin)/dmax
   20 continue
c
c     sort points into bins
c
      do 30 i=1,node
	  list(i)=i
   30 continue
      
c
c      call bsort(ktj,node,x,y,z,xmin,xmax,ymin,ymax,zmin,zmax,dmax
c     &          ,list,ibin)
c
c     compute delaunay triangulation
c
      call delaun(ktj,kte,node,nelm,x,y,z,list,mtj,jac,vx,vy,vz,rv
     &           ,kv,istack,map,err,ierrcode)
	if (ierrcode.ne.0) return
c
c     reset x,y,z-coords to original values
c
      do 40 i=1,node
	  x(i)=x(i)*dmax+xmin
	  y(i)=y(i)*dmax+ymin
	  z(i)=z(i)*dmax+zmin
   40 continue

      return
      end
c
c     ------   subroutine bsort   --------------------------------------
c
c     purpose : sort points such consecutive points are close to
c                one another in 3d space using a bin sort
c     last modified : 28 jan 1991
c
      subroutine bsort(ktj,node,x,y,z,xmin,xmax,ymin,ymax,zmin,zmax,dmax
     &                ,list,ibin)
      implicit real*8(a-h,o-z)
      dimension x(ktj+4),y(ktj+4),z(ktj+4),list(ktj),ibin(ktj)
c
c     compute number of bins
c     compute inverse of bin size
c
      ndiv=nint(real(node)**0.1d0)
      factx=real(ndiv)/((xmax-xmin)*1.01d0/dmax)
      facty=real(ndiv)/((ymax-ymin)*1.01d0/dmax)
      factz=real(ndiv)/((zmax-zmin)*1.01d0/dmax)
c
c     assign bin numbers to each point
c
      do 10 l=1,node
	ip=list(l)
	i=int(x(ip)*factx)
	j=int(z(ip)*factz)
	k=int(y(ip)*facty)
	if(mod(k,2).eq.0)then
	  if(mod(j,2).eq.0)then
	    ibin(ip)=k*ndiv*ndiv+j*ndiv+i+1
	  else
	    ibin(ip)=k*ndiv*ndiv+(j+1)*ndiv-i
	  end if
	else
	  if(mod(j,2).eq.0)then
	    ibin(ip)=k*ndiv*ndiv+(ndiv-j)*ndiv-i
	  else
	    ibin(ip)=k*ndiv*ndiv+(ndiv-j-1)*ndiv+i+1
	  end if
	end if
   10 continue
c
c     sort points in ascending sequence of bin number
c
      call qsorti(ktj,node,list,ibin)
c
      return
      end
c
c     ------   subroutine qsorti   -------------------------------------
c
c     purpose : order list of integers in ascending sequence of
c                their integer keys
c     last modified :  7 may 1985  s w sloan
c
      subroutine qsorti(k,n,list,key)
      implicit real*8(a-h,o-z)
      parameter(maxstk=32)
      dimension list(k),key(k),ilst(maxstk),irst(maxstk)
c
      ll=1
      lr=n
      istk=0
   10 if(ll.lt.lr)then
	nl=ll
	nr=lr
	lm=(ll+lr)/2
	iguess=key(list(lm))
c
c       find keys for exchange
c
   20   if(key(list(nl)).lt.iguess)then
	  nl=nl+1
	  go to 20
	end if
   30   if(iguess.lt.key(list(nr)))then
	  nr=nr-1
	  go to 30
	end if
	if(nl.lt.(nr-1))then
	  ltemp=list(nl)
	  list(nl)=list(nr)
	  list(nr)=ltemp
	  nl=nl+1
	  nr=nr-1
	  go to 20
	end if
c
c       deal with crossing of pointers
c
	if(nl.le.nr)then
	  if(nl.lt.nr)then
	    ltemp=list(nl)
	    list(nl)=list(nr)
	    list(nr)=ltemp
	  end if
	  nl=nl+1
	  nr=nr-1
	end if
c
c       select sub-list to be processed next
c
	istk=istk+1
	if(nr.lt.lm)then
	  ilst(istk)=nl
	  irst(istk)=lr
	  lr=nr
	else
	  ilst(istk)=ll
	  irst(istk)=nr
	  ll=nl
	end if
	go to 10
      end if
c
c     process any stacked sub-lists
c
      if(istk.ne.0)then
	ll=ilst(istk)
	lr=irst(istk)
	istk=istk-1
	go to 10
      end if
c
      return
      end
c
c     ------   subroutine delaun   -------------------------------------
c
c     purpose : compute 3d delaunay triangulation
c     last modified : 18 mar 1991
c
      subroutine delaun(ktj,kte,node,nelm,x,y,z,list,mtj,jac,vx,vy,vz,rv
     &                 ,kv,istack,map,err,ierrcode)
      implicit real*8(a-h,o-z)
      dimension x(ktj+4),y(ktj+4),z(ktj+4),list(ktj)
      dimension mtj(kte,4),jac(kte,4),vx(kte),vy(kte),vz(kte),rv(kte)
      dimension kv(kte),istack(kte),map(kte)

c      WRITE(*,*) 'START DELAUN()'
c
c     initialization
c
      do 10 i=1,kte
	  map(i)=0
   10 continue
c
c     setting of supertetrahedron
c
      nelm=1
      ia=ktj+1
      ib=ktj+2
      ic=ktj+3
      id=ktj+4
      mtj(1,1)=ia
      mtj(1,2)=ib
      mtj(1,3)=ic
      mtj(1,4)=id
      jac(1,1)=0
      jac(1,2)=0
      jac(1,3)=0
      jac(1,4)=0
c
      x(ia)=  8.66d0
      y(ia)=  0.00d0
      z(ia)= -5.00d0
      x(ib)= -4.33d0
      y(ib)=  7.50d0
      z(ib)= -5.00d0
      x(ic)= -4.33d0
      y(ic)= -7.50d0
      z(ic)= -5.00d0
      x(id)=  0.00d0
      y(id)=  0.00d0
      z(id)= 10.00d0
      vx(1)=  0.00d0
      vy(1)=  0.00d0
      vz(1)=  0.00d0
      rv(1)=100.00d0
c
c     loop over each point
c
      do 20 i=1,node
	  ip=list(i)
c        WRITE(*,*) 'I=',i,' IP=',ip, ' NELM=',nelm
	  xp=x(ip)
	  yp=y(ip)
        zp=z(ip)
c
c       search for tetrahedron which includes new data point
c
	  loc=locate(ktj,kte,xp,yp,zp,x,y,z,nelm,mtj,jac,err)
c
c       pause
c       search for all tetrahedra whose circumspheres
c        enclose new data point
c
	  iv=0
	  msk=0

	  iv=iv+1
	  kv(iv)=loc
	  map(loc)=1
	  msk=msk+1
	  istack(msk)=loc

   30   if(msk.ne.0)then
	    isk=istack(msk)
	    msk=msk-1
	    do 40 j=1,4
	      jelm=jac(isk,j)
	      if(jelm.eq.0)go to 40
	      if(map(jelm).eq.1)go to 40
	      rad=rv(jelm)*(1.d0+err)
	      dst=(vx(jelm)-xp)*(vx(jelm)-xp)
	      if(dst.ge.rad)go to 40
	      dst=dst+(vy(jelm)-yp)*(vy(jelm)-yp)
	      if(dst.ge.rad)go to 40
	      dst=dst+(vz(jelm)-zp)*(vz(jelm)-zp)
	      if(dst.ge.rad)go to 40
	      iv=iv+1
	      kv(iv)=jelm
	      map(jelm)=1
	      msk=msk+1
	      istack(msk)=jelm
   40       continue
	    go to 30
	  end if

c
c       triangulation of the polyhedron formed by tetrahedra
c        whose circumspheres enclose new data point
c
	  call poly(ktj,kte,ip,iv,kv,nelm,mtj,jac,vx,vy,vz,rv,x,y,z
     &            ,map,ierrcode)
	  if (ierrcode.ne.0) return

   20 continue

c
      if(nelm.gt.kte)then
        ierrcode=1
c	write(*,'('' ***error in subroutine delaun***'')')
c	write(*,'('' ***tetrahedra overflow***'')')
c	stop
      end if
c
c     remove all tetrahedra which include the forming points
c      of the supertetrahedron
c
      iv=0
      do 70 i=1,nelm
	  kv(i)=0
   70 continue
c
      do 80 i=1,nelm
	  if(mtj(i,1).gt.ktj)go to 90
	  if(mtj(i,2).gt.ktj)go to 90
	  if(mtj(i,3).gt.ktj)go to 90
	  if(mtj(i,4).gt.ktj)go to 90
	  go to 80
   90   iv=iv+1
	  kv(iv)=i
   80 continue
c
      call remove(kte,iv,kv,nelm,mtj,jac,vx,vy,vz,rv,map)
c
c     examine results of mesh generation
c
      call fill(kte,nelm,mtj,jac,ierrcode)
	if (ierrcode.ne.0) return
c

      return
      end
c
c     ------   function locate   ---------------------------------------
c
c     purpose : locate tetrahedron which encloses new data point
c     last modified :  6 feb 1990
c
      function locate(ktj,kte,xp,yp,zp,x,y,z,nelm,mtj,jac,err)
      implicit real*8(a-h,o-z)
      dimension x(ktj+4),y(ktj+4),z(ktj+4),mtj(kte,4),jac(kte,4)
c
      itet=nelm
   10 continue
      do 20 n=1,4
	  i=mtj(itet,mod(n,4)+1)
	  j=mtj(itet,4-(n-1)/2*2)
	  k=mtj(itet,3-mod(n/2,2)*2)
	  a=y(i)*z(j)+y(j)*z(k)+y(k)*z(i)-y(i)*z(k)-y(j)*z(i)-y(k)*z(j)
	  b=z(i)*x(j)+z(j)*x(k)+z(k)*x(i)-z(i)*x(k)-z(j)*x(i)-z(k)*x(j)
	  c=x(i)*y(j)+x(j)*y(k)+x(k)*y(i)-x(i)*y(k)-x(j)*y(i)-x(k)*y(j)
	  d=-a*x(i)-b*y(i)-c*z(i)
	  if(a*xp+b*yp+c*zp+d.lt.-err)then
	    itet=jac(itet,n)
	    go to 10
	  end if
   20 continue
c
c     tetrahedron has been found
c
      locate=itet
c
      return
      end
c
c     ------   subroutine poly   ---------------------------------------
c
c     purpose : remove edges interior to the polyhedron
c                and connect its vertices to new data point
c     last modified : 18 mar 1991
c
c     max surfaces : 5000
c
      subroutine poly(ktj,kte,ip,iv,kv,nelm,mtj,jac,vx,vy,vz,rv,x,y,z
     &               ,map,ierrcode)
      implicit real*8(a-h,o-z)
      parameter(ksf=1000)
      dimension x(ktj+4),y(ktj+4),z(ktj+4)
      dimension mtj(kte,4),jac(kte,4),vx(kte),vy(kte),vz(kte),rv(kte)
      dimension kv(kte),map(kte)
      dimension imen(ksf,3),jmen(ksf),kmen(ksf),vol(ksf)
c
c     search for boundary surfaces of the polyhedron
c
      
   10 ix=0
      do 20 i=1,iv
	ielm=kv(i)
	do 30 j=1,4
	  jelm=jac(ielm,j)
c
	  ia=mtj(ielm,mod(j,4)+1)
	  ib=mtj(ielm,4-(j-1)/2*2)
	  ic=mtj(ielm,3-mod(j/2,2)*2)
	  if(jelm.eq.0)then
	    ix=ix+1
	    imen(ix,1)=ia
	    imen(ix,2)=ib
	    imen(ix,3)=ic
	    jmen(ix)=0
	    kmen(ix)=0
	    vol(ix)=volume(ktj,ia,ib,ic,ip,x,y,z)
	  else if(map(jelm).eq.0)then
	    ix=ix+1
	    imen(ix,1)=ia
	    imen(ix,2)=ib
	    imen(ix,3)=ic
	    jmen(ix)=jelm
	    kmen(ix)=iface(kte,jelm,ielm,jac,ierrcode)
	    if (ierrcode.ne.0) return
	    vol(ix)=volume(ktj,ia,ib,ic,ip,x,y,z)
	    if(vol(ix).le.0.d0)then
	      iv=iv+1
	      kv(iv)=jelm
	      map(jelm)=1
	      go to 10
	    end if
	  end if
   30   continue
   20 continue

c
c     connect vertices of the surfaces to new data point
c
     
      ibound=ix
      do 40 i=iv+1,ibound
	  nelm=nelm+1
	  kv(i)=nelm
	  map(nelm)=1
   40 continue

c
      do 50 i=1,ibound
	  map(kv(i))=0
   50 continue
c
      do 60 i=1,ibound

	  ielm=kv(i)
	  determ=vol(i)
	  ia=imen(i,1)
	  ib=imen(i,2)
	  ic=imen(i,3)
	  mtj(ielm,1)=ia
	  mtj(ielm,2)=ib
	  mtj(ielm,3)=ic
	  mtj(ielm,4)=ip
	  jac(ielm,4)=jmen(i)

	  if(jmen(i).ne.0)then
	    jac(jmen(i),kmen(i))=ielm
	  end if

	  call sphere(ktj,ia,ib,ic,ip,determ,x,y,z,xv,yv,zv,rr)
	  vx(ielm)=xv
	  vy(ielm)=yv
	  vz(ielm)=zv
	  rv(ielm)=rr
   60 continue
c
c     connect tetrahedra in polyhedron each other
c
      
      ix=0
      do 70 i=1,ibound
	  ielm=kv(i)
	  do 80 j=1,3
	    ia=mtj(ielm,mod(j,3)+1)
	    ib=mtj(ielm,mod(mod(j,3)+1,3)+1)
	    do 90 k=1,ix
	      ja=imen(k,1)
	      jb=imen(k,2)
	      if((ia.eq.ja).and.(ib.eq.jb))then
	        jac(ielm,j)=jmen(k)
	        jac(jmen(k),kmen(k))=ielm
	        imen(k,1)=imen(ix,1)
	        imen(k,2)=imen(ix,2)
	        jmen(k)=jmen(ix)
	        kmen(k)=kmen(ix)
	        ix=ix-1
	        go to 80
	      end if
   90     continue
	    ix=ix+1
	    imen(ix,1)=ib
	    imen(ix,2)=ia
	    jmen(ix)=ielm
	    kmen(ix)=j
   80   continue
   70 continue
c
c     in case that new tetrahedra is less than old tetrahedra
c
      
      if(iv.gt.ibound)then
	ir=iv-ibound
	do 100 i=1,ir
	  kv(i)=kv(ibound+i)
	  map(kv(i))=kv(i)
  100   continue
	call qsorti(kte,ir,kv,map)
	do 110 i=1,ir
	  ielm=kv(ir-i+1)
	  map(ielm)=0
	  if(ielm.ne.nelm)then
	    vx(ielm)=vx(nelm)
	    vy(ielm)=vy(nelm)
	    vz(ielm)=vz(nelm)
	    rv(ielm)=rv(nelm)
	    do 120 j=1,4
	      mtj(ielm,j)=mtj(nelm,j)
	      jelm=jac(nelm,j)
	      jac(ielm,j)=jelm
	      if(jelm.ne.0)then
		jac(jelm,iface(kte,jelm,nelm,jac,ierrcode))=ielm
	      end if
  120       continue
	  end if
	  nelm=nelm-1
  110   continue
      end if
      return
      end
c
c     ------   function volume   ---------------------------------------
c
c     purpose : computation of volume of tetrahedron
c     last modified : 26 jan 1991
c
      function volume(ktj,ia,ib,ic,ip,x,y,z)
      implicit real*8(a-h,o-z)
      dimension x(ktj+4),y(ktj+4),z(ktj+4)
c
      xa=x(ia)
      ya=y(ia)
      za=z(ia)
      xb=x(ib)
      yb=y(ib)
      zb=z(ib)
      xc=x(ic)
      yc=y(ic)
      zc=z(ic)
      xp=x(ip)
      yp=y(ip)
      zp=z(ip)
      va=(xb-xa)*(yc-ya)*(zp-za)
      vb=(yb-ya)*(zc-za)*(xp-xa)
      vc=(zb-za)*(xc-xa)*(yp-ya)
      wa=(xb-xa)*(zc-za)*(yp-ya)
      wb=(yb-ya)*(xc-xa)*(zp-za)
      wc=(zb-za)*(yc-ya)*(xp-xa)
      volume=va+vb+vc-wa-wb-wc
c
      return
      end
c
c     ------   subroutine sphere   -------------------------------------
c
c     purpose : computation of circumsphere of tetrahedron
c     last modified : 16 oct 1990
c
      subroutine sphere(ktj,ia,ib,ic,ip,determ,x,y,z,xv,yv,zv,rr)
      implicit real*8(a-h,o-z)
      dimension x(ktj+4),y(ktj+4),z(ktj+4)
c
      xa=x(ia)
      ya=y(ia)
      za=z(ia)
      xb=x(ib)
      yb=y(ib)
      zb=z(ib)
      xc=x(ic)
      yc=y(ic)
      zc=z(ic)
      xp=x(ip)
      yp=y(ip)
      zp=z(ip)
c
c     cofactor
c
      p11=(yc-ya)*(zp-za)-(yp-ya)*(zc-za)
      p12=(xp-xa)*(zc-za)-(xc-xa)*(zp-za)
      p13=(xc-xa)*(yp-ya)-(xp-xa)*(yc-ya)
      p21=(yp-ya)*(zb-za)-(yb-ya)*(zp-za)
      p22=(xb-xa)*(zp-za)-(xp-xa)*(zb-za)
      p23=(xp-xa)*(yb-ya)-(xb-xa)*(yp-ya)
      p31=(yb-ya)*(zc-za)-(yc-ya)*(zb-za)
      p32=(xc-xa)*(zb-za)-(xb-xa)*(zc-za)
      p33=(xb-xa)*(yc-ya)-(xc-xa)*(yb-ya)
c
      xyza=xa*xa+ya*ya+za*za
      aa=0.5d0*(xb*xb+yb*yb+zb*zb-xyza)
      bb=0.5d0*(xc*xc+yc*yc+zc*zc-xyza)
      cc=0.5d0*(xp*xp+yp*yp+zp*zp-xyza)
      xv=(p11*aa+p21*bb+p31*cc)/determ
      yv=(p12*aa+p22*bb+p32*cc)/determ
      zv=(p13*aa+p23*bb+p33*cc)/determ
      rr=(xa-xv)*(xa-xv)+(ya-yv)*(ya-yv)+(za-zv)*(za-zv)
c
      return
      end
c
c     ------   subroutine remove   -------------------------------------
c
c     purpose : remove unnecessary tetrahedra
c     last modified :  1 feb 1990
c
      subroutine remove(kte,iv,kv,nelm,mtj,jac,vx,vy,vz,rv,map)
      implicit real*8(a-h,o-z)
      dimension kv(kte),mtj(kte,4),jac(kte,4),vx(kte),vy(kte),vz(kte)
      dimension rv(kte),map(kte)
c
c     initialization
c
      m=0
      n=0
      do 10 i=1,nelm
	map(i)=1
   10 continue
      do 20 i=1,iv
	map(kv(i))=0
   20 continue
c
      do 30 i=1,nelm
	if(map(i).ne.0)then
	  m=m+1
	  map(i)=m
	end if
   30 continue
c
      do 40 i=1,nelm
	if(map(i).ne.0)then
	  n=n+1
	  vx(n)=vx(i)
	  vy(n)=vy(i)
	  vz(n)=vz(i)
	  rv(n)=rv(i)
	  do 50 ia=1,4
	    mtj(n,ia)=mtj(i,ia)
	    if(jac(i,ia).eq.0)then
	      jac(n,ia)=0
	    else
	      jac(n,ia)=map(jac(i,ia))
	    end if
   50     continue
	end if
   40 continue
c
      do 60 i=n+1,nelm
	vx(i)=0.d0
	vy(i)=0.d0
	vz(i)=0.d0
	rv(i)=0.d0
	do 70 ia=1,4
	  mtj(i,ia)=0
	  jac(i,ia)=0
   70   continue
   60 continue
      nelm=nelm-iv
c
      return
      end
c
c     ------   subroutine circum   -------------------------------------
c
c     purpose : check if the generated tetrahedra are optimum
c     last modified : 25 oct 1990
c
      subroutine circum(ktj,kte,nelm,mtj,jac,x,y,z,vx,vy,vz,rv,err, 
     &                  ierrcode)
      implicit real*8(a-h,o-z)
      dimension mtj(kte,4),jac(kte,4),x(ktj+4),y(ktj+4),z(ktj+4)
      dimension vx(kte),vy(kte),vz(kte),rv(kte)
c
      do 10 i=1,nelm
	ielm=i
	va=vx(ielm)
	vb=vy(ielm)
	vc=vz(ielm)
	rmd=rv(ielm)*(1.d0-err)
	do 20 j=1,4
	  jelm=jac(i,j)
	  if(jelm.ne.0)then
	    ivop=mtj(jelm,iface(kte,jelm,ielm,jac,ierrcode))
	    dst=0.d0
	    wa=x(ivop)
	    wb=y(ivop)
	    wc=z(ivop)
	    dst=dst+(wa-va)*(wa-va)+(wb-vb)*(wb-vb)+(wc-vc)*(wc-vc)
	    if(dst.lt.rmd)then
	      ierrcode = 1
	      return
c	      write(*,'('' ***error in subroutine circum***'')')
c	      write(*,'('' ***node in a circumsphere***'')')
c	      stop
	    end if
	  end if
   20   continue
   10 continue
      return
      end
c
c     ------   subroutine fill   ---------------------------------------
c
c     purpose : check if the domain is filled by tetrahedra
c     last modified : 25 oct 1990
c
      subroutine fill(kte,nelm,mtj,jac,ierrcode)
      implicit real*8(a-h,o-z)
      dimension mtj(kte,4),jac(kte,4)
c
      do 10 i=1,nelm
	ielm=i
	do 20 j=1,4
	  ia=mtj(ielm,mod(j,4)+1)
	  ib=mtj(ielm,4-(j-1)/2*2)
	  ic=mtj(ielm,3-mod(j/2,2)*2)
	  jelm=jac(ielm,j)
	  if((jelm.ne.0).and.(ielm.lt.jelm))then
	    k=iface(kte,jelm,ielm,jac,ierrcode)
	    if (ierrcode.ne.0) return
	    ja=mtj(jelm,mod(k,4)+1)
	    jb=mtj(jelm,4-(k-1)/2*2)
	    jc=mtj(jelm,3-mod(k/2,2)*2)
	    if((ia.eq.jc).and.(ib.eq.jb).and.(ic.eq.ja))go to 20
	    if((ib.eq.jc).and.(ic.eq.jb).and.(ia.eq.ja))go to 20
	    if((ic.eq.jc).and.(ia.eq.jb).and.(ib.eq.ja))go to 20
	    ierrcode = 2
	    return
c	    write(*,'('' ***error in subroutine fill***'')')
c	    write(*,'('' ***tetrahedron not adjacent***'')')
c	    stop
	  end if
   20   continue
   10 continue
      return
      end
c
c     ------   function iface   ----------------------------------------
c
c     purpose : find surface in tet(l) which is adjacent to tet(k)
c     last modified : 27 oct 1990
c
      function iface(kte,l,k,jac,ierrcode)
      implicit real*8(a-h,o-z)
      dimension jac(kte,4)
c
      do 10 i=1,4
	if(jac(l,i).eq.k)then
	  iface=i
	  go to 20
	end if
   10 continue
      ierrcode = 3
c      write(*,'('' ***error in function iface***'')')
c      write(*,'('' ***tetrahedron not adjacent***'')')
c      stop
c
   20 return
      end
c
