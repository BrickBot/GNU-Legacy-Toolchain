      program rh233941
      implicit none
      complex*16 z
      z = dcmplx(1.0, 2.0)
      call sub(z)
      stop
      end program rh233941

      subroutine sub(z)
      implicit none
      complex*16 z
      z = dcmplx(-dimag(z), dreal(z))
      call sub2(z)
      return
      end subroutine sub

      subroutine sub2(z)
      implicit none
      complex*16 z
      if (dreal(z).ne.-2.0.or.dimag(z).ne.1.0) call abort
      end subroutine sub2
