package mainpackage;
import rs.etf.sab.operations.CityOperations;
import rs.etf.sab.tests.TestHandler;
import rs.etf.sab.tests.TestRunner;
import student.sn210229_CityOperations;
import student.sn210229_CourierOperations;
import student.sn210229_CourierRequestOperation;
import student.sn210229_DistrictOperations;
import student.sn210229_GeneralOperations;
import student.sn210229_PackageOperations;
import student.sn210229_UserOperations;
import student.sn210229_VehicleOperations;

public class StudentMain {

    public static void main(String[] args) {
        sn210229_CityOperations cityOperations = sn210229_CityOperations.getInstance();
        sn210229_DistrictOperations districtOperations = sn210229_DistrictOperations.getInstance();
        sn210229_CourierOperations courierOperations = sn210229_CourierOperations.getInstance();
        sn210229_CourierRequestOperation courierRequestOperation = sn210229_CourierRequestOperation.getInstance();
        sn210229_GeneralOperations generalOperations = sn210229_GeneralOperations.getInstance();
        sn210229_UserOperations userOperations = sn210229_UserOperations.getInstance();
        sn210229_VehicleOperations vehicleOperations = sn210229_VehicleOperations.getInstance();
        sn210229_PackageOperations packageOperations = sn210229_PackageOperations.getInstance();

        TestHandler.createInstance(
                cityOperations,
                courierOperations,
                courierRequestOperation,
                districtOperations,
                generalOperations,
                userOperations,
                vehicleOperations,
                packageOperations);

        TestRunner.runTests();
    }
}
