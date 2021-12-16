import * as UE from 'ue'
import {rpc, edit_on_instance} from 'ue'

console.warn("Script init of TsTestActor ");

class TsTestActor extends UE.Actor {
    static PrintVersion(){
        console.log(" version release");
    };
}

export default TsTestActor;
