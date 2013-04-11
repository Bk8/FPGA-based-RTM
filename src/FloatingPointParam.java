
public class FloatingPointParam
{
  public int getExponent()
  {
    return exponent;
  }

  public void setExponent(int exponent)
  {
    this.exponent = exponent;
  }

  public int getMantissa()
  {
    return mantissa;
  }

  public void setMantissa(int mantissa)
  {
    this.mantissa = mantissa;
  }

  private int exponent;
  private int mantissa;

  public FloatingPointParam(int exp, int man)
  {
    this.exponent = exp;
    this.mantissa = man;
  }
}
