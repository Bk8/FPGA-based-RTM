import com.maxeler.maxcompiler.v1.kernelcompiler.Kernel;
import com.maxeler.maxcompiler.v1.kernelcompiler.KernelParameters;
import com.maxeler.maxcompiler.v1.kernelcompiler.stdlib.core.CounterChain;
import com.maxeler.maxcompiler.v1.kernelcompiler.stdlib.core.Stream.OffsetExpr;
import com.maxeler.maxcompiler.v1.kernelcompiler.types.base.HWType;
import com.maxeler.maxcompiler.v1.kernelcompiler.types.base.HWVar;

public class PassThroughKernel extends Kernel {

	public PassThroughKernel(KernelParameters parameters, int nxMax,
	    long inx, long iny, long inz)
	{
    super(parameters);

    //HWType float_t = hwFloat(11, 53);
   HWType float_t = hwFloat(8, 24);
    HWType uint32_t = hwUInt(32);

    // Input
    HWVar Pt = io.input("Pt_stream", float_t);
    HWVar PtM1 = io.input("PtM1_stream", float_t);

    // offset expression
    OffsetExpr nx  = stream.makeOffsetParam("nx_offset", 3, nxMax);
    OffsetExpr nxy = stream.makeOffsetParam("nxy_offset", 3 * nx, nxMax * nx);

    // scalar input
    //    HWVar inx = io.scalarInput("nx", int32_t);
    //    HWVar iny = io.scalarInput("ny", int32_t);
    //    HWVar inz = io.scalarInput("nz", int32_t);

    HWVar w3 = io.scalarInput("w3", float_t);
    HWVar w2 = io.scalarInput("w2", float_t);
    HWVar w1 = io.scalarInput("w1", float_t);
    HWVar w0 = io.scalarInput("w0", float_t);
    HWVar dt = io.scalarInput("dt", float_t);
    HWVar c  = io.scalarInput("c",  float_t);
    HWVar dx = io.scalarInput("dx", float_t);
    HWVar A  = io.scalarInput("A" , float_t);


    // use chained counter to mimic the for loop
    CounterChain chain = control.count.makeCounterChain();
    HWVar iz = chain.addCounter(inz, 1);
    HWVar iy = chain.addCounter(iny, 1);
    HWVar ix = chain.addCounter(inx, 1);

    iz = iz.cast(uint32_t);
    iy = iy.cast(uint32_t);
    ix = ix.cast(uint32_t);


    HWVar in_middle = ((iz >=3) & (iz < inz - 3) &
                       (iy >=3) & (iy < iny - 3) &
                       (ix >=3) & (ix < inx - 3));

    HWVar tmp =
        (stream.offset(Pt, -3) + stream.offset(Pt, 3)) * w3 +
        (stream.offset(Pt, -2) + stream.offset(Pt, 2)) * w2 +
        (stream.offset(Pt, -1) + stream.offset(Pt, 1)) * w1 +

        (stream.offset(Pt, -3*nx) + stream.offset(Pt, 3*nx)) * w3 +
        (stream.offset(Pt, -2*nx) + stream.offset(Pt, 2*nx)) * w2 +
        (stream.offset(Pt, -1*nx) + stream.offset(Pt, 1*nx)) * w1 +

        (stream.offset(Pt, -3*nxy) + stream.offset(Pt, 3*nxy)) * w3 +
        (stream.offset(Pt, -2*nxy) + stream.offset(Pt, 2*nxy)) * w2 +
        (stream.offset(Pt, -1*nxy) + stream.offset(Pt, 1*nxy)) * w1 +

        (Pt * w0) * 3;

    HWVar PtStencil = in_middle ? tmp : Pt;

    HWVar PtP1 = (dt*dt) * ( (c*c / (dx*dx)) * PtStencil + A ) + 2*Pt - PtM1;

    // Output
    io.output("PtP1_stream", PtP1, float_t);
//    io.output("PtP1_stream", PtStencil, float_t);

	}
}
