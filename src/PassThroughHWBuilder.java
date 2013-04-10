import com.maxeler.maxcompiler.v1.kernelcompiler.Kernel;
import com.maxeler.maxcompiler.v1.managers.standard.Manager;
import com.maxeler.maxcompiler.v1.managers.standard.Manager.IOType;

public class PassThroughHWBuilder {
	  private static final int nxMax = 512;

	public static void main(String[] args) {
		Manager m = new Manager("PassThrough", BoardModel.BOARDMODEL);
		Kernel k = new PassThroughKernel( m.makeKernelParameters(), nxMax,
		    PassThroughSimRunner.nx,
		    PassThroughSimRunner.ny,
		    PassThroughSimRunner.nz);

		m.setKernel(k);

		m.setIO(IOType.ALL_PCIE);

		m.build();
	}
}
