import java.util.Random;

import com.maxeler.maxcompiler.v1.kernelcompiler.Kernel;
import com.maxeler.maxcompiler.v1.managers.standard.SimulationManager;

public class PassThroughSimRunner
{
  private static final double epsilon = 1e-10;
  private static final int nxMax = 512;
  private static final int dimension = 7;
  private final static int nx = dimension;
  private final static int ny = dimension;
  private final static int nz = dimension;
  private final static double w3 = (1.0 / 90.);
  private final static double w2 = (-3.0 / 20);
  private final static double w1 = 1.5;
  private final static double w0 = (-49.0 / 18);

  private final static double g_dt = 0.002;
  private final static double g_c = 3000;
  private final static double g_dx = 200;
  private final static double A = 10;

  private final static int n_iter = 1;

  private static double[] Pt_fpga = new double[nx * ny * nz];
  private static double[] PtM1_fpga = new double[nx * ny * nz];
  private static double[] PtP1_fpga;

  private static double[] Pt = new double[nx * ny * nz];
  private static double[] PtM1 = new double[nx * ny * nz];
  private static double[] PtP1 = new double[nx * ny * nz];
  private static double[] tmp = new double[nx * ny * nz];

  public static void main(String[] args)
  {

    initArray();
    // run in Java
    for (int i = 0; i < n_iter; i++) {
      acoustic(Pt, PtM1, A, PtP1);
      PtM1 = Pt;
      Pt = PtP1;
    }

    for (int i = 0; i < n_iter; i++) {
      SimulationManager m = new SimulationManager("PassThroughSim");
      Kernel k = new PassThroughKernel(m.makeKernelParameters(), nxMax,
          nx, ny, nz);

      // run in FPGA
      m.setKernel(k);

      // set offset parameters
      m.setStreamOffsetParam("nx_offset", nx);
      m.setStreamOffsetParam("nxy_offset", nx * ny);

      // set scalar input
      //      m.setScalarInput("nx", nx);
      //      m.setScalarInput("ny", ny);
      //      m.setScalarInput("nz", nz);
      m.setScalarInput("w3", w3);
      m.setScalarInput("w2", w2);
      m.setScalarInput("w1", w1);
      m.setScalarInput("w0", w0);
      m.setScalarInput("dt", g_dt);
      m.setScalarInput("dx", g_dx);
      m.setScalarInput("c", g_c);
      m.setScalarInput("A", A);
      m.setInputData("Pt_stream", Pt_fpga);
      m.setInputData("PtM1_stream", PtM1_fpga);
      m.setKernelCycles(nx * ny * nz);

      m.runTest();

      PtP1_fpga = m.getOutputDataArray("PtP1_stream");

      PtM1_fpga = Pt_fpga;
      Pt_fpga = PtP1_fpga;
    }

    // check output

    if (verifyAll(Pt_fpga, Pt)) {
      System.out.println("Test Pass!!!");
    } else {
      System.out.println("It is so sorry that the test Failed!!!!!!!!");
    }

  }

  private static void acoustic(double[] Pt, double[] PtM1, double A,
      double[] PtP1)
  {
    tmp = stencil_cpu(Pt, tmp);
    for (int i = 0; i < nx * ny * nz; i++) {
      PtP1[i] = (g_dt * g_dt) * ((g_c * g_c / (g_dx * g_dx)) * tmp[i] + A) + 2
        * Pt[i] - PtM1[i];
    }
  }

  private static boolean verify(double[] in, double[] out)
  {
    boolean ret = true;
    System.out.printf("(z,y,x)\toutput\t\texpected\n");
    for (int iz = 3; iz < nz - 3; iz++) {
      for (int iy = 3; iy < ny - 3; iy++) {
        for (int ix = 3; ix < nx - 3; ix++) {
          int index = at(iz, iy, ix);
          if (Math.abs(in[index] - out[index]) > epsilon) {
            System.out.printf("%d,%d,%d)\t%f\t%f\n", iz, iy, ix, in[index],
                out[index]);
            ret = false;
          }
        }
      }
    }
    return ret;
  }

  private static boolean verifyAll(double [] a, double [] b)
  {
    boolean ret = true;
    System.out.println("i:\t\toutput\t\t\texpected");
    for (int i = 0; i < nx * ny * nz; i++) {
      if (Math.abs(a[i] - b[i]) > epsilon) {
        int z = i / (nx * ny);
        int y = (i - z *nx*ny) / nx;
        int x = (i - z * ny * ny - y * nx);
        System.out.printf("%d: (%d,%d,%d):\t\t%f\t\t%f\n", i, z, y, x, a[i], b[i]);
        ret = false;
      }
    }
    return ret;
  }

  private static void initArray()
  {
    Random r = new Random();
    for (int i = 0; i < nx * ny * nz; i++) {
      Pt[i] = r.nextInt(1000000);
      PtM1[i] = r.nextInt(1000000);
      Pt_fpga[i] = Pt[i];
      PtM1_fpga[i] = PtM1[i];
    }
  }

  private static double[] stencil_cpu(double[] in, double[] out)
  {
    for (int iz = 0; iz < nz; iz++) {
      for (int iy = 0; iy < ny; iy++) {
        for (int ix = 0; ix < nx; ix++) {
          out[at(iz, iy, ix)] = stencil(in, iz, iy, ix);
        }
      }
    }

    return out;
  }

  private static int at(int z, int y, int x)
  {
    return z * nx * ny + y * nx + x;
  }

  private static double stencil(double[] in, int iz, int iy, int ix)
  {
    double xM3 = (ix - 3 < 0 ? 0 : in[at(iz, iy, ix - 3)]);
    double xM2 = (ix - 2 < 0 ? 0 : in[at(iz, iy, ix - 2)]);
    double xM1 = (ix - 1 < 0 ? 0 : in[at(iz, iy, ix - 1)]);
    double yM3 = (iy - 3 < 0 ? 0 : in[at(iz, iy - 3, ix)]);
    double yM2 = (iy - 2 < 0 ? 0 : in[at(iz, iy - 2, ix)]);
    double yM1 = (iy - 1 < 0 ? 0 : in[at(iz, iy - 1, ix)]);
    double zM3 = (iz - 3 < 0 ? 0 : in[at(iz - 3, iy, ix)]);
    double zM2 = (iz - 2 < 0 ? 0 : in[at(iz - 2, iy, ix)]);
    double zM1 = (iz - 1 < 0 ? 0 : in[at(iz - 1, iy, ix)]);

    double xP3 = (ix + 3 > nx - 1 ? 0 : in[at(iz, iy, ix + 3)]);
    double xP2 = (ix + 2 > nx - 1 ? 0 : in[at(iz, iy, ix + 2)]);
    double xP1 = (ix + 1 > nx - 1 ? 0 : in[at(iz, iy, ix + 1)]);
    double yP3 = (iy + 3 > ny - 1 ? 0 : in[at(iz, iy + 3, ix)]);
    double yP2 = (iy + 2 > ny - 1 ? 0 : in[at(iz, iy + 2, ix)]);
    double yP1 = (iy + 1 > ny - 1 ? 0 : in[at(iz, iy + 1, ix)]);
    double zP3 = (iz + 3 > nz - 1 ? 0 : in[at(iz + 3, iy, ix)]);
    double zP2 = (iz + 2 > nz - 1 ? 0 : in[at(iz + 2, iy, ix)]);
    double zP1 = (iz + 1 > nz - 1 ? 0 : in[at(iz + 1, iy, ix)]);

    return
      (xM3 + xP3) * w3 +
      (xM2 + xP2) * w2 +
      (xM1 + xP1) * w1 +

      (yM3 + yP3) * w3 +
      (yM2 + yP2) * w2 +
      (yM1 + yP1) * w1 +

      (zM3 + zP3) * w3 +
      (zM2 + zP2) * w2 +
      (zM1 + zP1) * w1 +

      (in[at(iz, iy, ix)] * w0) * 3;
  }
}
