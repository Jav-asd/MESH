MESH is written in an inheritance manner, so most of the functions in the base class can be directly accessed by subclasses. Usage of MESH involves writing a python script to call into various parts of MESH. Here we describe all of the MESH base class functions that can be called within the python environment.

For which functions can be called for a given geometry, please read the pages [SimulationPlanar](planar.md), [SimulationGrating](grating.md) and [SimulationPattern](pattern.md) for the geometries you are simulating.

!!! note
    The base class is just a wrapper for most of the functions, but it cannot be initiated in a python script. The only instances that can be initiated are the classes corresponding to different dimensions.

```python
AddMaterial(material name, input file)
```
* Arguments:
    1. material name: [string], the name of the material added to the simulation. Such name is unique and if there already exists a material with the same name, an error message will be printed out.
    2. input file: [string], a file that contains the dielectric properties of the corresponding material. For scalar dielectric, the input file should be formatted as  a list of        
    ```    
    omega eps_r eps_i      
    ```    
    For diagonal dielectric, the format is a list of        
    ```    
    omega eps_xx_r eps_xx_i eps_yy_r eps_yy_i eps_zz_r  eps_zz_i      
    ```    
    For tensor dielectric, the format is a list of     
    ```    
    omega eps_xx_r eps_xx_i eps_xy_r eps_xy_i eps_yx_r eps_yx_i eps_yy_r eps_yy_i eps_zz_r  eps_zz_i      
    ```     

* Output: None

* Note: The omega needs to be aligned for all the materials in the simulation.

```python
SetMaterial(material name, new epsilon)
```
* Arguments:
    1. material name: [string], the name of the material whose epsilon will be changed. This material should already exist in the simulation (by `AddMaterial`), otherwise an error will be printed out.
    2. new epsilon: [nested tuple], the length equals the number of omega, and per row is the epsilon values with the same format as the input of `AddMaterial` function. i.e. for scalar case the length will be $2$, for diagonal case the length is $6$ and for tensor case the length is $10$.

* Output: None

```python
AddLayer(layer name, thickness, material name)
```
* Arguments:
    1. layer name: [string], the name of the layer. Similarly, the names for layers are unique, and if such name already exists in the simulation, an error message will be printed out.
    2. thickness: [double], the thickness of the new layer in SI unit.
    3. material name: [string], the material that is used as the background of the layer. This material should already exist in the simulation (by `AddMaterial`), otherwise an error message will be printed out.

* Output: None

* Note: this new added layer will be placed on top of all the previous layers.

```python
SetLayer(layer name, thickness, material name)
```
* Arguments:
    1. layer name: [string], the layer whose thickness and background will be changed. Such layer needs to already exist in the simulation, otherwise an error message will be printed out.
    2. thickness: [double], the new thickness of the layer.
    3. material name: [string], the material for the background of the layer. If such material does not exist, an error message will be printed out.

* Output: None

```python
SetLayerThickness(layer name, thickness)
```
* Arguments:
    1. layer name: [string],  the layer whose thickness will be changed. Such layer needs to already exist in the simulation, otherwise an error message will be printed out.
    2. thickness: [double], the new thickness of the layer.

* Output: None  

```python
AddLayerCopy(new layer name, original layer name)
```
* Arguments:
    1. new layer name: [string], the new layer that is copied from the original layer.  Such name cannot already exist in the simulation, otherwise an error message will be printed out.
    2. original layer name: [string], the original layer from whom everything is copied. If this layer does not exist in the simulation, an error will be printed out.

* Output: None

* Note: this function only copies the structure information, for example any pattern of the original layer, but does not copy any thermal information. For example, even the original layer is set as a source, the copied layer is still not a source. In addition, this new added layer will be placed on top of all the previous layers.

```python
DeleteLayer(layer name)
```
* Arguments:
    1. layer name: [string], the name of the layer that will be deleted. Such layer should already be in the system, otherwise an error will be printed out.

* Output: None


```python
SetSourceLayer(layer name)
```
* Arguments:
    1. layer name: [string], the name of the layer that is designated as the source layer. Such layer should already exist in the system, otherwise an error will be printed out.

* Output: None

* Note: a system can have more than $1$ source layers.

```python
SetProbeLayer(layer name)
```
* Arguments:
    1. layer name: [string], the name of the layer that is designated as the probe layer of the flux. Such layer should already exist in the system, otherwise an error will be printed out.

* Output: None

* Note: a system can have only one probe layer. Setting another layer as the probe layer will overwrite the previous one. In addition, the probe layer should be above all the source layers in the real geometry.

```python
SetProbeLayerZCoordinate(target_z)
```
* Arguments:
    1. target_z: [double], the zth coordinate in the target layer where the Poynting flux is evaluated. By default this value is the thickness of the target layer

* Output: None


```python
SetThread(nthread)
```
* Arguments:
    1. nthreads: [int], number of threads used in OpenMP.

* Output: None
* Note: this function only works in an OpenMP setup.

```python
SetKxIntegral(points, end)
```
* Arguments:
    1. points: [int], number of points in the integration.
    2. end: [double, optional for grating and pattern geometries], the end of the integral over $k_x$. This end should be a normalized number with respect to $\omega/c$.

* Output: None

* Note: this function is essentially doing
$$\int_{-\text{end}\cdot \omega/c}^{\text{end}\cdot \omega/c}dk_x$$ where the integral is evaluated as a summation of `points` points. In the case when `end` is not given, the lower and upper bounds of the integral will be $\pm |G_1|/2$. where $G_1$ is the length of the reciprocal lattice with component in $x$ direction. See the following figure:
![reciprocal lattice](../LuaAPI/reciprocal.png)

```python
SetKyIntegral(points, end)
```
* Arguments:
    1. points: [int], number of points in the integration.
    2. end: [double, optional for pattern geometries], the end of the integral over $k_y$. This end should be a normalized number with respect to $\omega/c$.

* Output: None

* Note: this function is essentially doing
$$\int_{-\text{end}\cdot \omega/c}^{\text{end}\cdot \omega/c}dk_y$$ where the integral is evaluated as a summation of `points` points. In the case when `end` is not given, the lower and upper bounds of the integral will be $\pm |G_2|/2$, where $G_2$ is the length of the reciprocal lattice along $y$ direction

```python
SetKxIntegralSym(points, end)
```
* Arguments:
    1. points: [int], number of points in the integration.
    2. end: [double, optional for grating and pattern geometries], the end of the integral over $k_x$. This end should be a normalized number with respect to $\omega/c$.

* Output: None

* Note: this function is essentially doing
$$2\times \int_{0}^{\text{end}\cdot \omega/c}dk_x$$ where the integral is evaluated as a summation of `points` points. In the case when `end` is not given, the upper bound of the integral will be $|G_1|/2$.

```python
SetKyIntegralSym(points, end)
```
* Arguments:
    1. points: [int], number of points in the integration.
    2. end: [double, optional for pattern geometries], the end of the integral over $k_y$. This end should be a normalized number with respect to $\omega/c$.

* Output: None

* Note: this function is essentially doing
$$2\times \int_{0}^{\text{end}\cdot \omega/c}dk_y$$ where the integral is evaluated as a summation of `points` points. In the case when `end` is not given, the upper bound of the integral will be $|G_2|/2$.

```python
InitSimulation()
```
* Arguments: None

* Note: this function builds up the structure of the system.

```python
IntegrateKxKy()
```
* Arguments: None

* Output: None

* Note: this function integrates over $k_x$ and $k_y$ based on the integral properties set by the user. So the function can only be called after the $k_x$ and $k_y$ integrals are configured, and the system is initialized.

```python
IntegrateKxKyMPI(rank, size)
```
* Arguments:
    1. rank: [int], the rank of the thread.
    2. size: [int], the total size of the MPI run.

* Output: None

*  Note: this function can only be called during MPI. For an example of a funtion call,  please refer to [MPI example](../Examples/MPI.md).

```python
GetNumOfOmega()
```
* Arguments: None

* Output: [int], the number of total omega points computed in the simulation.

```python
GetPhi()
```
* Arguments: None

* Output: [tuple of double], the $\Phi(\omega)$ values obtained from the simulation.

* Note: can only be called after $k_x$ and $k_y$ are integrated.

```python
GetOmega()
```
* Arguments: None

* Output: [tuple of double], the omega values computed in the simulation.

```python
GetEpsilon(omega index, (x, y, z))
```
* Arguments:
    1. omega index: [int], the index of the omega value where $\epsilon$ is evaluated.  To be consistent with python, this index starts from $1$.
    2. (x, y, z): [double tuple], the real space position where the index is evaluated, in SI unit

* Output: value of epsilon with length $10$, in the form of
  ```
  eps_xx_r, eps_xx_i, eps_xy_r, eps_xy_i, eps_yx_r, eps_yx_i,　eps_yy_r, eps_yy_i, eps_zz_r, eps_zz_i
  ```
  These are computed by reconstruction of the Fourier series, so won't be the same as the exact dielectric function.


```python
GetPhiAtKxKy(omega index, kx, ky)
```
* Arguments:
    1. omega index: [int], the index of the omega value where $\Phi(\omega[\text{index}], k_x, k_y)$ is evaluated.  To be consistent with python, this index starts from $1$.
    2. kx: [double], the $k_x$ value where $\Phi(\omega[\text{index}], k_x, k_y)$ is evaluated. It is a normalized value by $\omega[\text{index}]/c$.
    3. ky: [double], the $k_y$ value where $\Phi(\omega[\text{index}], k_x, k_y)$ is evaluated. It is a normalized value by $\omega[\text{index}]/c$.

* Output: [double], the value of $\Phi(\omega, k_x, k_y)$.

```python
OutputSysInfo()
```
* Arguments: None

* Output: None

* Note: the function prints out a system description to screen.

```python
OutputLayerPatternRealization(omega index, name, Nu, Nv, filename)
```
* Arguments:
    1. omega index: [int], the index of the omega value that the dielectric is evaluated.   To be consistent with python, this index starts from $1$.
    2. name: [string], the name of the layer.
    3. Nu: [int], number of points in x direction in one periodicity.
    4. Nv: [int], number of points in y direction in one periodicity.
    5. filename: [string], optional. If not given, then the epsilon values will be printed to standard output.

* Output: None

* Note: the epsilon format will be the same as the function `GetEpsilon` along with the spacial coordinates.

Also, MESH provides some options for printing intermediate information and methods for Fourier transform of the dielectric.

```python
OptOnlyComputeTE()
```
* Arguments: None

* Output: None

* Note: with this function, the package only computes flux contributed from TE mode.

```python
OptOnlyComputeTM()
```
* Arguments: None

* Output: None

* Note: with this function, the package only computes flux contributed from TM mode.


```python
OptPrintIntermediate()
```
* Arguments: None

* Output: None

* Note: this function prints intermediate $\Phi(\omega, k_x, k_y)$ when function `IntegrateKxKy()` or `IntegrateKxKyMPI(rank, size)` is called. The output format is
a list of "$\omega$  $k_x$ $k_y$ $\Phi(\omega, k_x, k_y)$", where $k_x$ and $k_y$ are values normalized to $\omega/c$.

!!! warning
    In MPI this function is not guaranteed to be atomic, meaning the printed output might be messed because each process is writing its own value to the screen. However for OpenMP, there is no such problem.


MESH also provides physics constants to facilitate computation. The constant object can be initiated by
```python
from MESH import Constants
constant = Constants()
```

The supported constants are (all in SI unit)


* constant['pi']: the value of $\pi$.
* constant['k_B']: the value of $k_B$.
* constant['eps_0']: the value of $\epsilon_0$.
* constant['mu_0']: the value of $\mu_0$.
* constant['m_e']: the value of $m_e$, i.e. the mass of an electron.
* constant['eV']: electron volt in Joules.
* constant['h']: the value of Planck's constant.
* constant['h_bar']: the value of the reduced Planck's constant.
* constant['c_0']: the speed of light.
* constant['q']: the value of $q$, i.e. magnitude of electron charge.
* constant['sigma']: the value of $\sigma$, i.e. Stefan-Boltzmann constant.