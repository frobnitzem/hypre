c     
c=====================================================================
c     
c     routines to test matrices & print statistics
c     
c=====================================================================
c     
c---------------------------------------------------------------------
      subroutine fstats(nl,levels,idump,
     *     nun,imin,imax,a,ia,ja,iu,ip,icg,ifg,
     *     b,ib,jb,ipmn,ipmx,iv,xp,yp)
c     
c     
c     print matrix, interpolation statistics statistics
c     
c---------------------------------------------------------------------
c     
      implicit real*8 (a-h,o-z)
c     
      dimension imin(*),imax(*)
      dimension ia (*)
      dimension a  (*)
      dimension ja (*)
      dimension iu (*)
      dimension ip (*)
      dimension icg(*)
      dimension ifg(*)

      dimension ipmn(*),ipmx(*)
      dimension iv (*)
      real*8 xp (*)
      real*8 yp (*)

      dimension ib (*)
      dimension b  (*)
      dimension jb (*)
c     
      dimension iarr(10)
c     
c---------------------------------------------------------------------
c     
c===  > decode idump
c     
      call idec(idump,3,ndig,iarr)
c     
      iout  = iarr(1)
      kdump = iarr(2)
      ndump = iarr(3)

      if(iout.eq.0) return

c     
c     print matrix/interpolation info
c     

      write(6,4000)
      write(6,4001)
      do 20, k=1,nl
         call testsy(k,nun,imin,imax,a,ia,ja,iu)
 20   continue

      write(6,4002)
      write(6,4001)
      do 30, k=1,nl
         call testb(k,nun,imin,imax,b,ib,jb,iu,icg)
 30   continue

      do 40 k=1,nl
         if(k.eq.levels) iout=min0(iout,3)
         if(k.ge.kdump.and.k.le.kdump+ndump)
     *        call outa(k,iout,
     *        imin,imax,a,ia,ja,iu,ip,icg,ifg,
     *        b,ib,jb,ipmn,ipmx,iv,xp,yp)
 40   continue
      return
 4000 format(3x,' matrix    block     sizes',
     *     6 x,'   entries per row        block row sums'/
     *     3x,' k  i  j    rows   entries  sparse  ',
     *     'min   max  avg       min          max')
 4001 format(4x,72('='))
 4002 format(//'Interpolation statistics'/)
      end


c---------------------------------------------------------------------
c     
      subroutine testsy(k,nun,imin,imax,a,ia,ja,iu)
c     
c     test blocks
c     
c---------------------------------------------------------------------
c     
      implicit real*8 (a-h,o-z)
c     
      dimension imin(*),imax(*)
      dimension ia (*)
      dimension a  (*)
      dimension ja (*)
      dimension iu (*)
c     
      dimension rsmn(10,10),rsmx(10,10),npts(10),nnze(10,10)
      dimension ncmn(10,10),ncmx(10,10),rs(10),nc(10)
c     
c---------------------------------------------------------------------
c     
c     write(6,9876) k,imin(k),imax(k)
 9876 format('  testsy - k,imin,imax=',3i5)
      do 20 n1=1,nun
         npts(n1)=0
         do 10 n2=1,nun
            rsmn(n1,n2)=+1.e38
            rsmx(n1,n2)=-1.e38
            nnze(n1,n2)=0
            ncmn(n1,n2)=16384
            ncmx(n1,n2)=0
 10      continue
 20   continue
      ilo=imin(k)
      ihi=imax(k)
      do 60 i=ilo,ihi
         n1=iu(i)
         if(n1.le.0.or.n1.gt.nun) then
            write(6,'(/''  ERROR IN TESTSY -- IU('',i5,'')='',i5)') i,n1
            stop
         endif
         do 30 n=1,10
            rs(n)=0.e0
            nc(n)=0
 30      continue
         npts(n1)=npts(n1)+1
 9871    format('  testsy - i,iu,npts=',3i5)
         jlo=ia(i)
         jhi=ia(i+1)-1
         do 40 j=jlo,jhi
            ii=ja(j)
            if(j.eq.jlo.or.ii.ne.i) go to 39
            write(6,3900) i
 3900       format(/' ERROR: row ',i4,' contains duplicate entries')
 39         n2=iu(ii)
 9872       format('  snafu -- i,j,ii,iu(ii)=',4i4)
            rs(n2)=rs(n2)+a(j)
            nnze(n1,n2)=nnze(n1,n2)+1
            nc(n2)=nc(n2)+1
 40      continue
         do 50 n=1,nun
            if(abs(rs(n)).gt.rsmx(n1,n)) rsmx(n1,n)=abs(rs(n))
            if(abs(rs(n)).lt.rsmn(n1,n)) rsmn(n1,n)=abs(rs(n))
            if(nc(n).gt.ncmx(n1,n)) ncmx(n1,n)=nc(n)
            if(nc(n).lt.ncmn(n1,n)) ncmn(n1,n)=nc(n)
 50      continue
 60   continue
      do 70 n1=1,nun
         if(npts(n1).gt.0) nmax=n1
 70   continue
      do 90 n1=1,nmax
         if(npts(n1).eq.0) go to 90
         do 80 n2=1,nmax
            if(npts(n2).eq.0) go to 90
            rsav=float(nnze(n1,n2))/float(npts(n1))
            spar=float(nnze(n1,n2))/float(npts(n1)**2)
            write(6,1000) k,n1,n2,npts(n1),nnze(n1,n2),spar,ncmn(n1,n2),
     *           ncmx(n1,n2),rsav,rsmn(n1,n2),rsmx(n1,n2)
 80      continue
 90   continue
      return
 1000 format(3x,i2,2(2x,i1),3x,i5,3x,i7,3x,
     *     f5.3,1x,i3,3x,i3,1x,f5.1,2(3x,e10.3))
      end

c==============================================================
c     
      subroutine testb(k,nun,imin,imax,b,ib,jb,iu,icg)
      implicit real*8 (a-h,o-z)
c     
c---------------------------------------------------------------------
c     
c     test interpolation
c     
c---------------------------------------------------------------------
c     
      dimension imin(*),imax(*)
      dimension b  (*)
      dimension ib (*)
      dimension jb (*)
      dimension iu (*)
      dimension icg(*)
c     
      dimension wmn(10),wmx(10),wsmn(10),wsmx(10)
      dimension npts(10),nimn(10),nimx(10)
c     
c---------------------------------------------------------------------
c     
 9876 format('  testb  - k,imin,imax=',3i5)
c     
c     initialize stats
c     
      do 20 n1=1,nun
         npts(n1)=0
         wmn(n1)=+1.e38
         wmx(n1)=-1.e38
         wsmn(n1)=+1.e38
         wsmx(n1)=-1.e38
         npts(n1)=0
         nimn(n1)=999
         nimx(n1)=0
 20   continue
      
     i        lo=imin(k)
      ihi=imax(k)
      nmax=0
c     
c     loop over rows of fine-grid matrix
c     skip if a coarse-grid point (icg(i).ge.0)
c     
      do 60 i=ilo,ihi
         if(icg(i).ge.0) go to 60
         n1=iu(i)
         npts(n1)=npts(n1)+1
         if(n1.gt.nmax) nmax=n1
         if(n1.le.0.or.n1.gt.nun) then
            write(6,'(/''  ERROR IN TESTB -- IU('',i5,'')='',i5)') i,n1
            stop
         endif
         jlo=ib(i)
         jhi=ib(i+1)-1
         ws=0.0
         nip=jhi-jlo+1
c     
c     find smallest # interpolation points 
c     and largest # of interpolation points
c     
         if(nip.lt.nimn(n1)) nimn(n1)=nip
         if(nip.gt.nimx(n1)) nimx(n1)=nip

         do 40 j=jlo,jhi
            ii=jb(j)
            n2=iu(ii)
            ws=ws+b(j)
            if(b(j).lt.wmn(n1)) wmn(n1)=b(j)
            if(b(j).gt.wmx(n1)) wmx(n1)=b(j)
 40      continue
         if(ws.lt.wsmn(n1)) wsmn(n1)=ws
         if(ws.gt.wsmx(n1)) wsmx(n1)=ws
 60   continue

      do 90 n1=1,nmax
         write(6,1000) k,n1,npts(n1),nimn(n1),nimx(n1),
     *        wmn(n1),wmx(n1),wsmn(n1),wsmx(n1)
 90   continue

      return
 1000 format(3x,i2,2x,i1,2x,i5,2i3,3x,4(2x,d9.3))
      end
