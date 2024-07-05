
package student;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import rs.etf.sab.operations.DistrictOperations;

/**
 *
 * @author nikol
 */
public class sn210229_DistrictOperations implements DistrictOperations
{

    private static Connection conn = null;
    private static sn210229_DistrictOperations inst = null;
    
    private sn210229_DistrictOperations(){
        conn = Database.getInstance().getConnection();
    }
    
    public static sn210229_DistrictOperations getInstance(){
        if (inst == null) inst = new sn210229_DistrictOperations();
        return inst;
    }
    
    @Override
    public int insertDistrict(String name, int cityId, int xCord, int yCord) { 
        if(sn210229_CityOperations.getInstance().getAllCities().contains(cityId)==false)return -1;
        String query = "insert into Opstina (Naziv, IdG, X, Y)" +
                " values (?, ?, ?, ?)";
        try {
            PreparedStatement ps = conn.prepareStatement(query, PreparedStatement.RETURN_GENERATED_KEYS);
            ps.setString(1, name);
            ps.setInt(2, cityId);
            ps.setInt(3, xCord);
            ps.setInt(4, yCord);
            ps.executeUpdate();
            ResultSet rs = ps.getGeneratedKeys();
            if(rs.next()){
                return rs.getInt(1);
            }
            return -1;
        } catch (SQLException ex) {
            ex.printStackTrace();
            return -1;
        }
    }
    private int deleteDistrict(String name){ 
        String query = "select Id from Opstina where Naziv = ?";
        int districtId = -1;
        int numOfDeleted = 0;
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1,name);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                districtId = rs.getInt(1);
                numOfDeleted += 1;
                deleteDistrict(districtId);
            }
        } catch (Exception e) {e.printStackTrace();}
        return numOfDeleted;
    }

    @Override
    public boolean deleteDistrict(int districtId) { 
        if(getAllDistricts().contains(districtId)==false)return false;
        try {
            String query3 = "delete from Opstina where Id = ?";
            PreparedStatement ps3 = conn.prepareStatement(query3,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps3.setInt(1,districtId);
            ps3.executeUpdate();
            return true;
        } catch (Exception e) {e.printStackTrace();}
        return true;
    }

    @Override
    public int deleteAllDistrictsFromCity(String nameOfTheCity) { 
        int countOfDeleted = 0;
        String query = "select Opstina.Id from Opstina join Grad on Opstina.IdG = Grad.Id where Grad.Naziv = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1, nameOfTheCity);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                deleteDistrict(rs.getInt(1));
                countOfDeleted+=1;
            }
        }catch (Exception e) {e.printStackTrace();return 0;}
        return countOfDeleted;
    }

    @Override
    public List<Integer> getAllDistricts() { 
        ArrayList<Integer> rez = new ArrayList<>();
        String query = "select * from Opstina";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getInt(1));
            }

        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }
    
    @Override
    public List<Integer> getAllDistrictsFromCity(int cityId) { 
        ArrayList<Integer> rez = new ArrayList<>();
        String query = "select * from Opstina where IdG = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, cityId);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getInt(1));
            }

        }catch (Exception e) {e.printStackTrace();return null;}
        if(rez.isEmpty()){
            return null;
        }else{
            return rez;
        }
    }
    
    @Override
    public int deleteDistricts(String... strings) { 
        int rez = 0;
        for (String s : strings) {
            rez+= deleteDistrict(s);
        }
        return rez;
    }
}
