import com.maxeler.maxcompiler.v1.kernelcompiler.Kernel;
import com.maxeler.maxcompiler.v1.kernelcompiler.KernelParameters;
import com.maxeler.maxcompiler.v1.kernelcompiler.stdlib.core.Count;
import com.maxeler.maxcompiler.v1.kernelcompiler.stdlib.core.Count.Counter;
import com.maxeler.maxcompiler.v1.kernelcompiler.stdlib.core.Stream.OffsetExpr;
import com.maxeler.maxcompiler.v1.kernelcompiler.types.base.HWType;
import com.maxeler.maxcompiler.v1.kernelcompiler.types.base.HWVar;

public class PassThroughKernel extends Kernel
{
  // floating point type. The bit of exponent and mantissa is passed through constructor
  private final HWType float_t;

  private static final int counterBitWidth = 32;
  private final HWType uint32_t = hwUInt(32);
  private HWVar Pt; // current wave field at time t
  private HWVar PtM1; // previous wave field at time (t-1)
  private OffsetExpr nx; // # of elements in x-axis
  private OffsetExpr nxy; // # of elements in the x-y plane, usually (nx * ny)
  private HWVar w3; // coefficient w3
  private HWVar w2; // coefficient w2
  private HWVar w1; // coefficient w1
  private HWVar w0; // coefficient w0
  private HWVar dt; // delta t
  private HWVar c ; // velocity
  private HWVar dx; // delta x
  private HWVar counterXMax; // the max value of counter calculating x-axis, usually equals nx
  private HWVar counterYMax; // the max value of counter calculating y-axis, usually equals ny
  private HWVar counterZMax; // the max value of counter calculating z-axis, usually equals nz
  private Counter counterX; // counter for the x-axis
  private Counter counterY; // counter for the y-axis
  private Counter counterZ; // counter for the z-axis


  public PassThroughKernel(KernelParameters   parameters,
                           int                nxMax,
                           FloatingPointParam fpp)
  {
    super(parameters);

    float_t = hwFloat(fpp.getExponent(), fpp.getMantissa());

    // get external input
    getInput();
    getScalarInput();
    getRunTimeParams(nxMax);

    // use advanced counter to mimic the for loop
    makeZYXCounter();

    HWVar iz = counterZ.getCount();
    HWVar iy = counterY.getCount();
    HWVar ix = counterX.getCount();

    HWVar PtStencil = inTheMiddle(iz, iy, ix) ? stencilArithmetic() : Pt;

    HWVar PtP1 = (dt*dt) * ((c*c/(dx*dx))*PtStencil) + 2*Pt - PtM1;

    // Output
    io.output("PtP1_stream", PtP1, float_t);
  }

  private void getInput()
  {
    Pt   = io.input("Pt_stream", float_t);
    PtM1 = io.input("PtM1_stream", float_t);
  }

  private void getScalarInput()
  {
    counterXMax = io.scalarInput("xMax", uint32_t);
    counterYMax = io.scalarInput("yMax", uint32_t);
    counterZMax = io.scalarInput("zMax", uint32_t);
    w3          = io.scalarInput("w3", float_t);
    w2          = io.scalarInput("w2", float_t);
    w1          = io.scalarInput("w1", float_t);
    w0          = io.scalarInput("w0", float_t);
    dt          = io.scalarInput("dt", float_t);
    c           = io.scalarInput("c", float_t);
    dx          = io.scalarInput("dx", float_t);
  }

  private void getRunTimeParams(int nxMax)
  {
    nx  = stream.makeOffsetParam("nx_offset", 3, nxMax);
    nxy = stream.makeOffsetParam("nxy_offset", 3 * nx, nxMax * nx);
  }

  private void makeZYXCounter()
  {
    // make counter controlling x axis
    Count.Params counterXParam = control.count.makeParams(counterBitWidth);
    counterXParam = counterXParam.withMax(counterXMax);
    counterX = control.count.makeCounter(counterXParam);

    // make counter controlling y axis
    Count.Params counterYParam = control.count.makeParams(counterBitWidth);
    counterYParam = counterYParam.withMax(counterYMax);
    counterYParam = counterYParam.withEnable(counterX.getWrap());
    counterY = control.count.makeCounter(counterYParam);

    // make counter controlling z axis
    Count.Params counterZParam = control.count.makeParams(counterBitWidth);
    counterZParam = counterZParam.withMax(counterZMax);
    counterZParam = counterZParam.withEnable(counterY.getWrap());
    counterZ = control.count.makeCounter(counterZParam);
  }

  private HWVar stencilArithmetic()
  {
    return
      (stream.offset(Pt, -3) + stream.offset(Pt, 3)) * w3 +
      (stream.offset(Pt, -2) + stream.offset(Pt, 2)) * w2 +
      (stream.offset(Pt, -1) + stream.offset(Pt, 1)) * w1 +

      (stream.offset(Pt, -3 * nx) + stream.offset(Pt, 3 * nx)) * w3 +
      (stream.offset(Pt, -2 * nx) + stream.offset(Pt, 2 * nx)) * w2 +
      (stream.offset(Pt, -1 * nx) + stream.offset(Pt, 1 * nx)) * w1 +

      (stream.offset(Pt, -3 * nxy) + stream.offset(Pt, 3 * nxy)) * w3 +
      (stream.offset(Pt, -2 * nxy) + stream.offset(Pt, 2 * nxy)) * w2 +
      (stream.offset(Pt, -1 * nxy) + stream.offset(Pt, 1 * nxy)) * w1 +

      (Pt * w0) * 3;
  }

  private HWVar inTheMiddle(HWVar iz, HWVar iy, HWVar ix)
  {
    return ((iz >= 3) & (iz < counterZMax - 3) & (iy >= 3)
        & (iy < counterYMax - 3) & (ix >= 3) & (ix < counterXMax - 3));
  }

}
