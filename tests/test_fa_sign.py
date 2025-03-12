import pytest

from application_client.aptos_command_sender import AptosCommandSender, Errors
from application_client.aptos_response_unpacker import unpack_get_public_key_response, unpack_sign_tx_response
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns
from utils import ROOT_SCREENSHOT_PATH, check_signature_validity

from aptos_sdk.transactions import RawTransaction, TransactionPayload, TransactionArgument, EntryFunction
from aptos_sdk.account import AccountAddress
from aptos_sdk.type_tag import StructTag, TypeTag
from aptos_sdk.bcs import Serializer


def prepare_tx():
  transmitter = "0x8F13f355F3aF444BD356ADEAAAF01235A7817D6A4417F5c9FA3D74A68F7b7AFD"
  destination = "0x7be51d04d3a482fa056bc094bc5eadad005aaf823a95269410f08730f0d03cb4"
  sender = AccountAddress.from_str(transmitter)
  receiver =  AccountAddress.from_str(destination)

  contractAddress = AccountAddress.from_str("0x7be51d04d3a482fa056bc094bc5eadad005aaf823a95269410f08730f0d03cb5")

  send_amount = 1000000
  # Transaction parameters
  sequence_number = 0
  expiration_timestamp = 0
  payload = EntryFunction.natural(
      "0x1::primary_fungible_store",
      "transfer",
      [TypeTag(StructTag.from_str("0x1::fungible_asset::Metadata"))],
      [
          #TransactionArgument(Serializer.sequence([contractAddress], Serializer.struct)),
          TransactionArgument(contractAddress, Serializer.struct),
          TransactionArgument(receiver, Serializer.struct),
          TransactionArgument(send_amount, Serializer.u64),
      ],
  )

  # Create the raw transaction (TX_RAW)
  fees = 900
  max_gas_amount = 1100
  gas_unit_price = int(fees/max_gas_amount)
  txn = RawTransaction(
      sender=sender,
      sequence_number=sequence_number,
      payload=TransactionPayload(payload),
      max_gas_amount=max_gas_amount,
      gas_unit_price=gas_unit_price,
      expiration_timestamps_secs=expiration_timestamp,
      chain_id=1,
  )
  # Serialize the transaction
  serializer = Serializer()
  txn.serialize(serializer)
  transaction = serializer.output()
  print("Transaction: ", str(transaction))
  # This is a salt required by the Nano App to make sure that the payload comes from Ledger Live host
  transaction = bytes.fromhex("b5e97db07fa0bd0e5598aa3643a9bc6f6693bddc1a9fec9e674a461eaa00b193" + transaction.hex())
  return transaction


# In this test we send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
def test_sign_fa_tx(firmware, backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = AptosCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44'/637'/1'/0'/0'"

    # First we need to get the public key of the device in order to build the transaction
    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    # Create the transaction that will be sent to the device for signing
    transaction = prepare_tx()

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_tx(path=path, transaction=transaction):
        # Validate the on-screen request by performing the navigation appropriate for this device
        if firmware.device.startswith("nano"):
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                      [NavInsID.BOTH_CLICK],
                                                      "Approve",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
        else:
            navigator.navigate_until_text_and_compare(NavInsID.USE_CASE_VIEW_DETAILS_NEXT,
                                                      [NavInsID.USE_CASE_REVIEW_CONFIRM,
                                                       NavInsID.USE_CASE_STATUS_DISMISS],
                                                      "Hold to sign",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    _, sig, _ = unpack_sign_tx_response(response)
    assert check_signature_validity(public_key, sig, transaction)