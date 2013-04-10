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

    HWType float_t = hwFloat(11, 53);
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



    // M: minus, zM3 <==> z-3
    // P: plus
    HWVar xM3 = (ix - 3 < 0 ? 0 : stream.offset(Pt, -3    ));
    HWVar xM2 = (ix - 2 < 0 ? 0 : stream.offset(Pt, -2    ));
    HWVar xM1 = (ix - 1 < 0 ? 0 : stream.offset(Pt, -1    ));
    HWVar yM3 = (iy - 3 < 0 ? 0 : stream.offset(Pt, -3*nx ));
    HWVar yM2 = (iy - 2 < 0 ? 0 : stream.offset(Pt, -2*nx ));
    HWVar yM1 = (iy - 1 < 0 ? 0 : stream.offset(Pt, -1*nx ));
    HWVar zM3 = (iz - 3 < 0 ? 0 : stream.offset(Pt, -3*nxy));
    HWVar zM2 = (iz - 2 < 0 ? 0 : stream.offset(Pt, -2*nxy));
    HWVar zM1 = (iz - 1 < 0 ? 0 : stream.offset(Pt, -1*nxy));

    HWVar xP3 = (ix + 3 > inx - 1 ? 0 : stream.offset(Pt, 3    ));
    HWVar xP2 = (ix + 2 > inx - 1 ? 0 : stream.offset(Pt, 2    ));
    HWVar xP1 = (ix + 1 > inx - 1 ? 0 : stream.offset(Pt, 1    ));
    HWVar yP3 = (iy + 3 > iny - 1 ? 0 : stream.offset(Pt, 3*nx ));
    HWVar yP2 = (iy + 2 > iny - 1 ? 0 : stream.offset(Pt, 2*nx ));
    HWVar yP1 = (iy + 1 > iny - 1 ? 0 : stream.offset(Pt, 1*nx ));
    HWVar zP3 = (iz + 3 > inz - 1 ? 0 : stream.offset(Pt, 3*nxy));
    HWVar zP2 = (iz + 2 > inz - 1 ? 0 : stream.offset(Pt, 2*nxy));
    HWVar zP1 = (iz + 1 > inz - 1 ? 0 : stream.offset(Pt, 1*nxy));

    // calculate the stencil
    HWVar PtStencil = (xM3 + xP3) * w3 +
                 (xM2 + xP2) * w2 +
                 (xM1 + xP1) * w1 +

                 (yM3 + yP3) * w3 +
                 (yM2 + yP2) * w2 +
                 (yM1 + yP1) * w1 +

                 (zM3 + zP3) * w3 +
                 (zM2 + zP2) * w2 +
                 (zM1 + zP1) * w1 +

                 (Pt * w0) * 3;


    HWVar PtP1 = (dt*dt) * ( (c*c / (dx*dx)) * PtStencil + A ) + 2*Pt - PtM1;

		// Output
		io.output("PtP1_stream", PtP1, float_t);
	}
}
