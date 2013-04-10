import com.maxeler.maxcompiler.v1.kernelcompiler.Kernel;
import com.maxeler.maxcompiler.v1.managers.MAX3BoardModel;
import com.maxeler.maxcompiler.v1.managers.standard.Manager;
import com.maxeler.maxcompiler.v1.managers.standard.Manager.IOType;

public class PassThroughHostSimBuilder {
  private static final int nxMax = 64;
  public static final int dimension = 8;
  public final static int nx = dimension;
  public final static int ny = dimension;
  public final static int nz = dimension;

  public static void main(String[] args) {
    Manager m = new Manager(true,"PassThroughHostSim", MAX3BoardModel.MAX3424A);
    Kernel k  = new PassThroughKernel( m.makeKernelParameters("PassThroughKernel"),
                                      nxMax, nx, ny, nz);

    m.setKernel(k);
    m.setIO(IOType.ALL_PCIE);
    m.build();
  }

}
