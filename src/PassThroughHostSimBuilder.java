import com.maxeler.maxcompiler.v1.kernelcompiler.Kernel;
import com.maxeler.maxcompiler.v1.managers.standard.Manager;
import com.maxeler.maxcompiler.v1.managers.standard.Manager.IOType;

public class PassThroughHostSimBuilder {
  private static final int nxMax = 512;

  public static void main(String[] args) {
    Manager m = new Manager(true,"PassThroughHostSim", BoardModel.BOARDMODEL);
    Kernel k  = new PassThroughKernel( m.makeKernelParameters("PassThroughKernel"), nxMax );

    m.setKernel(k);

    m.setIO(IOType.ALL_PCIE);
    m.build();
  }

}
